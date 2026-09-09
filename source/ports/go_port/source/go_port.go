/*
 *	MetaCall Go Port by Parra Studios
 *	A frontend for Go language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

package metacall

/*
#cgo CFLAGS: -Wall
#cgo LDFLAGS: -lmetacall

#include <metacall/metacall.h>

// TODO: Sanitizer
// #if defined(__ADDRESS_SANITIZER__) || defined(__THREAD_SANITIZER__) || defined(__MEMORY_SANITIZER__)
// void __lsan_do_leak_check(void);
// #endif

// Since main.go has //export directives we can't place function definitions in
// it - we'll get multiple definition errors from the linker (see
// https://golang.org/cmd/cgo/#hdr-C_references_to_Go for more on this
// limitation). We can't mark them 'static inline' either because we're taking
// their address to pass to clibrary; thus, they are moved to a separate Go
// file.

// Based on: https://eli.thegreenplace.net/2019/passing-callbacks-and-pointers-to-cgo/

extern void *resolveCgo(void *, void *);
extern void *rejectCgo(void *, void *);

*/
import "C"

import (
	"bytes"
	"context"
	"errors"
	"fmt"
	"reflect"
	"runtime"
	"sync"
	"unsafe"
)

const QUEUEBUFFSIZE = 1

// interface for all works with execute and cancel functions
type safeWork interface {
	execute()
	cancel(err error)
}
type callReturnSafeWork struct {
	value interface{}
	err   error
}
type loadFromFileSafeWork struct {
	tag     string
	scripts []string
	err     chan error
}

func (w *loadFromFileSafeWork) execute() {
	err := LoadFromFileUnsafe(w.tag, w.scripts)
	w.err <- err
}
func (w *loadFromFileSafeWork) cancel(err error) {
	w.err <- err
}

type loadFromMemorySafeWork struct {
	tag    string
	buffer string
	err    chan error
}

func (w *loadFromMemorySafeWork) execute() {
	err := LoadFromMemoryUnsafe(w.tag, w.buffer)
	w.err <- err
}
func (w *loadFromMemorySafeWork) cancel(err error) {
	w.err <- err
}

type callSafeWork struct {
	function string
	args     []interface{}
	ret      chan callReturnSafeWork
}

func (w *callSafeWork) execute() {
	value, err := CallUnsafe(w.function, w.args...)
	w.ret <- callReturnSafeWork{value, err}
}
func (w *callSafeWork) cancel(err error) {
	w.ret <- callReturnSafeWork{nil, err}
}

type awaitCallback func(interface{}, interface{}) interface{}

type awaitSafeWork struct {
	function string
	args     []interface{}
	ret      chan callReturnSafeWork
	resolve  awaitCallback
	reject   awaitCallback
	ctx      interface{}
}

func (w *awaitSafeWork) execute() {
	value, err := AwaitUnsafe(w.function, w.resolve, w.reject, w.ctx, w.args...)
	w.ret <- callReturnSafeWork{value, err}
}
func (w *awaitSafeWork) cancel(err error) {
	w.ret <- callReturnSafeWork{nil, err}
}

type awaitCallbacks struct {
	resolve awaitCallback
	reject  awaitCallback
	ctx     interface{}
}

const PtrSizeInBytes = (32 << uintptr(^uintptr(0)>>63)) >> 3

var (
	queue       = make(chan safeWork, QUEUEBUFFSIZE) // Queue for dispatching the work
	toggle      chan struct{}                        // Channel for stopping the queue
	lock        sync.Mutex                           // Lock for the queue
	rootCtx     context.Context                      // root context to manage cancelation
	rootCancel  context.CancelFunc                   // cancel function to drain and destroy jobs in queue
	errShutdown = errors.New("runtime shutdown")     // err to be sent to cancel() when shutting down
)

func InitializeUnsafe() error {
	// TODO: Sanitizer
	// C.__lsan_do_leak_check()

	// TODO: Remove this once go loader is implemented
	if result := int(C.metacall_initialize()); result != 0 {
		return fmt.Errorf("initializing MetaCall (error code %d)", result)
	}

	return nil
}

// Start starts the metacall adapter
func Initialize() error {
	lock.Lock()
	defer lock.Unlock()

	if rootCtx != nil {
		// Already running
		return nil
	}

	rootCtx, rootCancel = context.WithCancel(context.Background())
	toggle = make(chan struct{}, 1)
	initErr := make(chan error, 1)

	go func(ctx context.Context, initErr chan error) {
		defer close(toggle)
		// Bind this goroutine to its thread
		runtime.LockOSThread()
		defer runtime.UnlockOSThread()

		// Initialize MetaCall
		if err := InitializeUnsafe(); err != nil {
			initErr <- err
			return
		}

		close(initErr)

		for {
			select {
			case <-ctx.Done():
				// Shutdown
				for {
					select {
					//drain queue
					case w := <-queue:
						w.cancel(errShutdown)
					// call destroy for metacall after queue is empty
					default:
						DestroyUnsafe()
						return
					}
				}
			// execute work in queue
			case w := <-queue:
				w.execute()
			}
		}
	}(rootCtx, initErr)

	return <-initErr
}

func LoadFromFileUnsafe(tag string, scripts []string) error {
	size := len(scripts)

	if size == 0 {
		return fmt.Errorf("Failed to load scripts of length 0 with tag %s", tag)
	}

	cTag := C.CString(tag)
	defer C.free(unsafe.Pointer(cTag))

	cScripts := C.malloc(C.size_t(size) * C.size_t(unsafe.Sizeof((*C.char)(nil))))
	defer C.free(unsafe.Pointer(cScripts))

	// Convert cScripts to a Go Array so we can index it
	goScripts := unsafe.Slice((**C.char)(unsafe.Pointer(cScripts)), size)

	for index, script := range scripts {
		goScripts[index] = C.CString(script)
	}

	defer func() {
		for i := 0; i < size; i++ {
			C.free(unsafe.Pointer(goScripts[i]))
		}
	}()

	if int(C.metacall_load_from_file(cTag, (**C.char)(unsafe.Pointer(cScripts)), (C.size_t)(size), nil)) != 0 {
		return fmt.Errorf("%s loader failed to load a script from the list: %v", tag, scripts)
	}

	return nil
}

func LoadFromMemoryUnsafe(tag string, buffer string) error {
	size := len(buffer) + 1

	cTag := C.CString(tag)
	defer C.free(unsafe.Pointer(cTag))

	cBuffer := C.CString(buffer)
	defer C.free(unsafe.Pointer(cBuffer))

	if int(C.metacall_load_from_memory(cTag, cBuffer, (C.size_t)(size), nil)) != 0 {
		return fmt.Errorf("%s loader failed to load a script from the buffer: %s", tag, buffer)
	}

	return nil
}

func CallUnsafe(function string, args ...interface{}) (interface{}, error) {
	cFunc, err := getFunction(function)
	if err != nil {
		return nil, err
	}

	length := C.size_t(len(args))
	cArgs := C.malloc(length * C.size_t(unsafe.Sizeof(uintptr(0))))

	for index, arg := range args {
		goToValue(arg, (*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(cArgs))+uintptr(index)*PtrSizeInBytes)))
	}

	defer func() {
		for index, _ := range args {
			C.metacall_value_destroy(*(*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(cArgs)) + uintptr(index)*PtrSizeInBytes)))
		}

		C.free(unsafe.Pointer(cArgs))
	}()

	ret := C.metacallfv_s(cFunc, (*unsafe.Pointer)(cArgs), length)

	if ret != nil {
		defer C.metacall_value_destroy(ret)
		return valueToGo(ret), nil
	}

	return nil, nil
}

// Call sends work and blocks until it's processed
func Call(function string, args ...interface{}) (interface{}, error) {
	ctx := checkRootCtx()
	if ctx == nil {
		return nil, errShutdown
	}

	ret := make(chan callReturnSafeWork, 1)
	w := &callSafeWork{
		function: function,
		args:     args,
		ret:      ret,
	}
	// if ctx canceled return, otherwise send work to queue
	select {
	case <-ctx.Done():
		return nil, errShutdown
	case queue <- w:
	}
	// if ctx canceled return, otherwise return finished work from ret chan
	// seperated frow above switch cause of returning values
	select {
	case <-ctx.Done():
		return nil, errShutdown
	case result := <-ret:
		return result.value, result.err
	}
}

//export goResolve
func goResolve(v unsafe.Pointer, ctx unsafe.Pointer) unsafe.Pointer {
	var ptr unsafe.Pointer
	goCb := pointerGet(ctx).(*awaitCallbacks)
	defer pointerDelete(ctx)
	goToValue(goCb.resolve(valueToGo(v), goCb.ctx), &ptr)

	return ptr
}

//export goReject
func goReject(v unsafe.Pointer, ctx unsafe.Pointer) unsafe.Pointer {
	var ptr unsafe.Pointer
	goCb := pointerGet(ctx).(*awaitCallbacks)
	defer pointerDelete(ctx)
	goToValue(goCb.reject(valueToGo(v), goCb.ctx), &ptr)

	return ptr
}

func AwaitUnsafe(function string, resolve, reject awaitCallback, ctx interface{}, args ...interface{}) (interface{}, error) {
	cFunc, err := getFunction(function)
	if err != nil {
		return nil, err
	}

	length := C.size_t(len(args))
	cArgs := C.malloc(length * C.size_t(unsafe.Sizeof(uintptr(0))))

	for index, arg := range args {
		goToValue(arg, (*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(cArgs))+uintptr(index)*PtrSizeInBytes)))
	}

	defer func() {
		for index, _ := range args {
			C.metacall_value_destroy(*(*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(cArgs)) + uintptr(index)*PtrSizeInBytes)))
		}

		C.free(unsafe.Pointer(cArgs))
	}()

	cCallbacks := C.metacall_await_callbacks{}

	if resolve != nil {
		cCallbacks.resolve = C.metacall_await_callback(C.resolveCgo)
	} else {
		cCallbacks.resolve = nil
	}

	if reject != nil {
		cCallbacks.reject = C.metacall_await_callback(C.rejectCgo)
	} else {
		cCallbacks.reject = nil
	}

	goCallbacks := awaitCallbacks{
		resolve: resolve,
		reject:  reject,
		ctx:     ctx,
	}

	goCallbacksPtr := pointerSave(&goCallbacks)

	ret := C.metacallfv_await_struct_s(cFunc, (*unsafe.Pointer)(cArgs), length, cCallbacks, goCallbacksPtr)

	if ret != nil {
		defer C.metacall_value_destroy(ret)
		return valueToGo(ret), nil
	} else {
		// delete and free ptr if metacallfv_await_struct_s failed with nil
		pointerDelete(goCallbacksPtr)
	}

	return nil, nil
}

// Await sends asynchronous work and blocks until it's processed
func Await(function string, resolve, reject awaitCallback, ctx interface{}, args ...interface{}) (interface{}, error) {
	rCtx := checkRootCtx()
	if rCtx == nil {
		return nil, errShutdown
	}

	ret := make(chan callReturnSafeWork, 1)

	w := &awaitSafeWork{
		function: function,
		args:     args,
		ret:      ret,
		resolve:  resolve,
		reject:   reject,
		ctx:      ctx,
	}

	select {
	case <-rCtx.Done():
		return nil, errShutdown
	case queue <- w:
	}

	select {
	case <-rCtx.Done():
		return nil, errShutdown
	case result := <-ret:
		return result.value, result.err
	}
}

func getFunction(function string) (unsafe.Pointer, error) {
	cFunction := C.CString(function)
	defer C.free(unsafe.Pointer(cFunction))
	cFunc := C.metacall_function(cFunction)
	if cFunc == nil {
		return nil, errors.New("function not found: " + function)
	}
	return cFunc, nil
}

func goToValue(arg interface{}, ptr *unsafe.Pointer) {
	// Create null
	if arg == nil {
		*ptr = C.metacall_value_create_null()
		return
	}

	// Create bool
	if i, ok := arg.(bool); ok {
		if i {
			*ptr = C.metacall_value_create_bool(C.uchar(1))
			return
		} else {
			*ptr = C.metacall_value_create_bool(C.uchar(0))
			return
		}
	}

	// Create char
	if i, ok := arg.(byte); ok {
		*ptr = C.metacall_value_create_char((C.char)(i))
		return
	}

	// Create short
	if i, ok := arg.(int16); ok {
		*ptr = C.metacall_value_create_short((C.short)(i))
		return
	}

	// Create int
	if i, ok := arg.(int); ok {
		*ptr = C.metacall_value_create_int((C.int)(i))
		return
	}

	// Create long
	if i, ok := arg.(int64); ok {
		*ptr = C.metacall_value_create_long((C.long)(i))
		return
	}

	// Create float32
	if i, ok := arg.(float32); ok {
		*ptr = C.metacall_value_create_float((C.float)(i))
		return
	}

	// Create float64
	if i, ok := arg.(float64); ok {
		*ptr = C.metacall_value_create_double((C.double)(i))
		return
	}

	// Create string
	if str, ok := arg.(string); ok {
		cStr := C.CString(str)
		defer C.free(unsafe.Pointer(cStr))
		*ptr = C.metacall_value_create_string(cStr, (C.size_t)(len(str)))
		return
	}

	if buf, ok := arg.(bytes.Buffer); ok {
		str := buf.String()
		p := unsafe.Pointer(C.CString(str))
		defer C.free(p)

		*ptr = C.metacall_value_create_buffer(p, (C.size_t)(len(str)))

		return
	}

	// Create array
	v := reflect.ValueOf(arg)
	if v.Kind() == reflect.Slice || v.Kind() == reflect.Array {
		length := v.Len()
		*ptr = C.metacall_value_create_array(nil, (C.size_t)(length))
		cArgs := C.metacall_value_to_array(*ptr)
		for index := 0; index < length; index++ {
			goToValue(v.Index(index).Interface(), (*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(cArgs))+uintptr(index)*PtrSizeInBytes)))
		}
		return
	}

	// Create map
	if v.Kind() == reflect.Map {
		length := v.Len()
		cArgs := C.malloc(C.size_t(length) * C.size_t(unsafe.Sizeof(uintptr(0))))
		defer C.free(unsafe.Pointer(cArgs))

		for index, m := 0, v.MapRange(); m.Next(); index++ {
			pair := [2]interface{}{m.Key().Interface(), m.Value().Interface()}

			goToValue(pair, (*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(cArgs))+uintptr(index)*PtrSizeInBytes)))
		}
		*ptr = C.metacall_value_create_map((*unsafe.Pointer)(cArgs), (C.size_t)(length))
		return
	}

	/*

		// Create map
		if v.Kind() == reflect.Map {
			length := v.Len()
			*ptr = C.metacall_value_create_map(nil, (C.size_t)(length))
			cArgs := C.metacall_value_to_map(*ptr)

			for index, m := 0, v.MapRange(); m.Next(); index++ {
				// Access to current element of the map
				mapIndex := unsafe.Pointer(uintptr(unsafe.Pointer(cArgs))+uintptr(index)*PtrSizeInBytes)

				// Get the map pair
				array := C.metacall_value_to_array(mapIndex)

				// Transform the key
				key := (*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(array))+uintptr(0)*PtrSizeInBytes))
				goToValue(m.Key(), key)

				// Transform the value
				val := (*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(array))+uintptr(1)*PtrSizeInBytes))
				goToValue(m.Value(), val)
			}
			return
		}

	*/

	// TODO: Add more types

	*ptr = nil
}

func valueToGo(value unsafe.Pointer) interface{} {
	switch C.metacall_value_id(value) {
	case C.METACALL_NULL:
		{
			return nil
		}
	case C.METACALL_BOOL:
		{
			return C.metacall_value_to_bool(value) != C.uchar(0)
		}
	case C.METACALL_CHAR:
		{
			return byte(C.metacall_value_to_char(value))
		}
	case C.METACALL_SHORT:
		{
			return int16(C.metacall_value_to_short(value))
		}
	case C.METACALL_INT:
		{
			return int(C.metacall_value_to_int(value))
		}
	case C.METACALL_LONG:
		{
			return int64(C.metacall_value_to_long(value))
		}
	case C.METACALL_FLOAT:
		{
			return float32(C.metacall_value_to_float(value))
		}

	case C.METACALL_DOUBLE:
		{
			return float64(C.metacall_value_to_double(value))
		}

	case C.METACALL_STRING:
		{
			return C.GoString(C.metacall_value_to_string(value))
		}
	case C.METACALL_BUFFER:
		{
			buffer := C.metacall_value_to_buffer(value)
			size := C.metacall_value_size(value)

			b := *bytes.NewBuffer(C.GoBytes(buffer, C.int(size)))

			return b
		}
	case C.METACALL_ARRAY:
		{
			arrayValue := C.metacall_value_to_array(value)
			arraySize := C.metacall_value_count(value)
			array := make([]interface{}, arraySize)

			for iterator := C.size_t(0); iterator < arraySize; iterator++ {
				currentValue := (*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(arrayValue)) + uintptr(iterator*PtrSizeInBytes)))
				array[iterator] = valueToGo(*currentValue)
			}

			return array
		}
	case C.METACALL_MAP:
		{
			tuples := C.metacall_value_to_map(value)
			size := C.metacall_value_count(value)

			m := make(map[string]interface{}, size)
			for i := C.size_t(0); i < size; i++ {
				pair := (*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(tuples)) + uintptr(i*PtrSizeInBytes)))
				p := reflect.ValueOf(valueToGo(*pair))

				key := p.Index(0).Interface().(string)
				m[key] = p.Index(1).Interface()
			}

			return m
		}

		// TODO: Add more types
	}
	return nil
}

func LoadFromFile(tag string, scripts []string) error {
	ctx := checkRootCtx()
	if ctx == nil {
		return errShutdown
	}

	result := make(chan error, 1)
	w := &loadFromFileSafeWork{
		tag,
		scripts,
		result,
	}
	select {
	case <-ctx.Done():
		return errShutdown
	case queue <- w:
	}

	select {
	case <-ctx.Done():
		return errShutdown
	case res := <-result:
		return res
	}
}

func LoadFromMemory(tag string, buffer string) error {
	ctx := checkRootCtx()
	if ctx == nil {
		return errShutdown
	}

	result := make(chan error, 1)
	w := &loadFromMemorySafeWork{
		tag,
		buffer,
		result,
	}

	select {
	case <-ctx.Done():
		return errShutdown
	case queue <- w:
	}

	select {
	case <-ctx.Done():
		return errShutdown
	case res := <-result:
		return res
	}
}

func checkRootCtx() context.Context {
	// lock to prevent race condition when accessing global variable rootCtx
	lock.Lock()
	// reject call when Initialize() was never called or Destroy() ran or in progress
	if rootCtx == nil || rootCtx.Err() != nil {
		lock.Unlock()
		return nil
	}
	ctx := rootCtx
	lock.Unlock()
	return ctx
}
func DestroyUnsafe() {
	C.metacall_destroy()
}

// Shutdown disables the metacall adapter waiting for all calls to complete
func Destroy() {
	lock.Lock()
	if rootCancel != nil {
		// cancel root ctx and assign it to nil when shutting down
		rootCancel()
		rootCancel = nil
		rootCtx = nil
	}
	tog := toggle
	lock.Unlock()

	if tog != nil {
		// wait for DestroyUnsafe() to finish
		<-tog
	}
}
