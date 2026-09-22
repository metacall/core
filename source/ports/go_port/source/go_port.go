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

// Since go already have its own sanitizers we don't need to use c sanitizers in go port
// as it will conflict with c sanitizers runtime and cause segfault. also c sanitizers can't
// identify goroutines leak, deadlocks and can false positive memory created by c and passed
// to go as a leak so it's better to use go sanitizers.
// to use go sanitizers use the following flags with go commands (build, test, ...):
// -race -> flag for thread sanitizer | -asan -> for address sanitizer | -msan -> for memeort sanitizer

// Since main.go has //export directives we can't place function definitions in
// it - we'll get multiple definition errors from the linker (see
// https://golang.org/cmd/cgo/#hdr-C_references_to_Go for more on this
// limitation). We can't mark them 'static inline' either because we're taking
// their address to pass to clibrary; thus, they are moved to a separate Go
// file.

// Based on: https://eli.thegreenplace.net/2019/passing-callbacks-and-pointers-to-cgo/

static inline void *metacall_error_throw_msg(const char *label, const char *message) {
	return metacall_error_throw(label, 0, "", "%s", message);
}

extern void *resolveCgo(void *, void *);
extern void *rejectCgo(void *, void *);

*/
import "C"

import (
	"bytes"
	"context"
	"errors"
	"fmt"
	"math"
	"net/http"
	"net/http/pprof"
	"os"
	"reflect"
	"runtime"
	"sync"
	"unsafe"

	// library to provide a symbolic backtrace of cgo functions	to help debugging and monitoring of c functions
	// this library works with pprof of go
	_ "github.com/ianlancetaylor/cgosymbolizer"
	"github.com/joho/godotenv"
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

type loadFromPackageSafeWork struct {
	tag  string
	pack string
	err  chan error
}

func (w *loadFromPackageSafeWork) execute() {
	err := LoadFromPackageUnsafe(w.tag, w.pack)
	w.err <- err
}
func (w *loadFromPackageSafeWork) cancel(err error) {
	w.err <- err
}

type loadFromPackageExSafeWork struct {
	tag     string
	pack    string
	options interface{}
	err     chan error
}

func (w *loadFromPackageExSafeWork) execute() {
	err := LoadFromPackageExUnsafe(w.tag, w.pack, w.options)
	w.err <- err
}
func (w *loadFromPackageExSafeWork) cancel(err error) {
	w.err <- err
}

type loadFromConfigSafeWork struct {
	config string
	err    chan error
}

func (w *loadFromConfigSafeWork) execute() {
	err := LoadFromConfigUnsafe(w.config)
	w.err <- err
}
func (w *loadFromConfigSafeWork) cancel(err error) {
	w.err <- err
}

type executionPathSafeWork struct {
	tag  string
	path string
	err  chan error
}

func (w *executionPathSafeWork) execute() {
	err := ExecutionPathUnsafe(w.tag, w.path)
	w.err <- err
}
func (w *executionPathSafeWork) cancel(err error) {
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
	// TODO: Remove this once go loader is implemented
	if result := int(C.metacall_initialize()); result != 0 {
		return fmt.Errorf("initializing MetaCall (error code %d)", result)
	}

	return nil
}

// Start starts the metacall adapter
func Initialize() error {
	// load .env file and start profile server only in debug mode
	_ = godotenv.Load()
	mode := os.Getenv("MODE")
	if mode == "debug" {
		startProfilesServer()
	}

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

func LoadFromPackageUnsafe(tag string, pack string) error {
	cTag := C.CString(tag)
	defer C.free(unsafe.Pointer(cTag))

	cPack := C.CString(pack)
	defer C.free(unsafe.Pointer(cPack))

	if int(C.metacall_load_from_package(cTag, cPack, nil)) != 0 {
		return fmt.Errorf("%s loader failed to load from package: %s", tag, pack)
	}

	return nil
}

func LoadFromPackageExUnsafe(tag string, pack string, options interface{}) error {
	cTag := C.CString(tag)
	defer C.free(unsafe.Pointer(cTag))

	cPack := C.CString(pack)
	defer C.free(unsafe.Pointer(cPack))

	// Convert options to metacall value (need goToValue for map/array)
	var optVal unsafe.Pointer
	if options != nil {
		goToValue(options, &optVal)
		defer C.metacall_value_destroy(optVal)
	}

	if int(C.metacall_load_from_package_ex(cTag, cPack, nil, optVal)) != 0 {
		return fmt.Errorf("%s loader failed to load from package: %s", tag, pack)
	}
	return nil
}

func LoadFromConfigUnsafe(config string) error {
	cConfig := C.CString(config)
	defer C.free(unsafe.Pointer(cConfig))

	if int(C.metacall_load_from_configuration(cConfig, nil, nil)) != 0 {
		return fmt.Errorf("failed to load config: %s", config)
	}

	return nil
}

func LoadFromConfigExUnsafe(config string) error {
	cConfig := C.CString(config)
	defer C.free(unsafe.Pointer(cConfig))

	// if int(C.metacall_load_from_configuration_export(cConfig, nil, nil)) != 0 {
	// 	return fmt.Errorf("failed to load config: %s", config)
	// }

	return nil
}

func ExecutionPathUnsafe(tag string, path string) error {
	cTag := C.CString(tag)
	defer C.free(unsafe.Pointer(cTag))

	cPath := C.CString(path)
	defer C.free(unsafe.Pointer(cPath))

	if int(C.metacall_execution_path(cTag, cPath)) != 0 {
		return fmt.Errorf("%s failed to load from execution path: %s", tag, path)
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
		id := C.metacall_value_id(ret)
		// calling value destroy on these types make the ptr in them a dangling pointer which causes a segfault when dereferencing
		if id != C.METACALL_CLASS && id != C.METACALL_OBJECT && id != C.METACALL_FUTURE {
			defer C.metacall_value_destroy(ret)
		}

		v := valueToGo(ret)
		if err, ok := v.(error); ok {
			return nil, err
		}
		return v, nil
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
		id := C.metacall_value_id(ret)
		// calling value destroy on these types make the ptr in them a dangling pointer which causes a segfault when dereferencing
		if id != C.METACALL_CLASS && id != C.METACALL_OBJECT && id != C.METACALL_FUTURE {
			defer C.metacall_value_destroy(ret)
		}
		v := valueToGo(ret)
		if err, ok := v.(error); ok {
			return nil, err
		}
		return v, nil
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
	// type-switch is performance wise from multiple if statements
	switch i := arg.(type) {
	// Create null
	case nil:
		*ptr = C.metacall_value_create_null()

	// Create bool
	case bool:
		if i {
			*ptr = C.metacall_value_create_bool(C.uchar(1))
		} else {
			*ptr = C.metacall_value_create_bool(C.uchar(0))
		}

	// Create char
	case byte:
		*ptr = C.metacall_value_create_char((C.char)(i))

	// Create short
	case int16:
		*ptr = C.metacall_value_create_short((C.short)(i))

	// create int from int32
	case int32:
		*ptr = C.metacall_value_create_int((C.int)(i))

	// Create int
	case int:
		// check if it is 32 or 64 bit
		if i >= math.MinInt32 && i <= math.MaxInt32 {
			*ptr = C.metacall_value_create_int((C.int)(i))
		} else {
			*ptr = C.metacall_value_create_long((C.long)(i))
		}

	// Create long
	case int64:
		*ptr = C.metacall_value_create_long((C.long)(i))

	// Create float32
	case float32:
		*ptr = C.metacall_value_create_float((C.float)(i))

	// Create float64
	case float64:
		*ptr = C.metacall_value_create_double((C.double)(i))

	// Create string
	case string:
		cStr := C.CString(i)
		defer C.free(unsafe.Pointer(cStr))
		*ptr = C.metacall_value_create_string(cStr, (C.size_t)(len(i)))

	// Create buffer
	case bytes.Buffer:
		str := i.String()
		p := unsafe.Pointer(C.CString(str))
		defer C.free(p)

		*ptr = C.metacall_value_create_buffer(p, (C.size_t)(len(str)))

	// Create pointer
	case unsafe.Pointer:
		*ptr = C.metacall_value_create_ptr(i)

	// Create exception
	case error:
		cMsg := C.CString(i.Error())
		cLabel := C.CString("Error")

		defer C.free(unsafe.Pointer(cMsg))
		defer C.free(unsafe.Pointer(cLabel))

		*ptr = C.metacall_error_throw_msg(cLabel, cMsg)

	// create class
	case *Class:
		if i.val != nil {
			// increment internal refrence so caller cleanup do not destroy go instance
			*ptr = C.metacall_value_copy(i.val)
		} else {
			*ptr = C.metacall_value_create_class(i.ptr)
		}

	// create object
	case *Object:
		if i.val != nil {
			// increment internal refrence so caller cleanup do not destroy go instance
			*ptr = C.metacall_value_copy(i.val)
		} else {
			*ptr = C.metacall_value_create_object(i.ptr)
		}

	// create future
	case *Future:
		if i.val != nil {
			// increment internal refrence so caller cleanup do not destroy go instance
			*ptr = C.metacall_value_copy(i.val)
		} else {
			*ptr = C.metacall_value_create_future(i.ptr)
		}

	default:
		v := reflect.ValueOf(arg)

		switch v.Kind() {
		// Create array
		case reflect.Slice, reflect.Array:
			length := v.Len()
			*ptr = C.metacall_value_create_array(nil, (C.size_t)(length))
			cArgs := C.metacall_value_to_array(*ptr)
			for index := 0; index < length; index++ {
				goToValue(v.Index(index).Interface(), (*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(cArgs))+uintptr(index)*PtrSizeInBytes)))
			}

		// Create map
		case reflect.Map:
			length := v.Len()
			cArgs := C.malloc(C.size_t(length) * C.size_t(unsafe.Sizeof(uintptr(0))))
			defer C.free(unsafe.Pointer(cArgs))

			for index, m := 0, v.MapRange(); m.Next(); index++ {
				pair := [2]interface{}{m.Key().Interface(), m.Value().Interface()}

				goToValue(pair, (*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(cArgs))+uintptr(index)*PtrSizeInBytes)))
			}
			*ptr = C.metacall_value_create_map((*unsafe.Pointer)(cArgs), (C.size_t)(length))

		default:
			*ptr = nil
		}
	}
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
	case C.METACALL_PTR:
		{
			return C.metacall_value_to_ptr(value)
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
	case C.METACALL_CLASS:
		{
			return newClass(value)
		}
	case C.METACALL_OBJECT:
		{
			return newObject(value, nil)
		}
	case C.METACALL_FUTURE:
		{
			return newFuture(value)
		}
	case C.METACALL_EXCEPTION:
		{
			var exception C.struct_metacall_exception_type

			if C.metacall_error_from_value(value, &exception) == 0 {
				return errors.New(C.GoString(exception.label) + " : " + C.GoString(exception.message))
			}
			return errors.New("Metacall Exception: UNKNOWN")
		}
	case C.METACALL_THROWABLE:
		{
			throw := C.metacall_value_to_throwable(unsafe.Pointer(value))
			val := C.metacall_throwable_value(throw)
			v := valueToGo(val)

			if err, ok := v.(error); ok {
				return err
			}
			// for languages that do not throw an error like JS
			e := fmt.Errorf("%v", v)
			return e
		}
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

func LoadFromPackage(tag string, pack string) error {
	ctx := checkRootCtx()
	if ctx == nil {
		return errShutdown
	}

	result := make(chan error, 1)
	w := &loadFromPackageSafeWork{
		tag:  tag,
		pack: pack,
		err:  result,
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

func LoadFromPackageEx(tag string, pack string, options interface{}) error {
	ctx := checkRootCtx()
	if ctx == nil {
		return errShutdown
	}

	result := make(chan error, 1)
	w := &loadFromPackageExSafeWork{
		tag:     tag,
		pack:    pack,
		options: options,
		err:     result,
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

func LoadFromConfig(config string) error {
	ctx := checkRootCtx()
	if ctx == nil {
		return errShutdown
	}

	result := make(chan error, 1)
	w := &loadFromConfigSafeWork{
		config: config,
		err:    result,
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

func ExecutionPath(tag string, path string) error {
	ctx := checkRootCtx()
	if ctx == nil {
		return errShutdown
	}

	result := make(chan error, 1)
	w := &executionPathSafeWork{
		tag:  tag,
		path: path,
		err:  result,
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
	// Run GC and wait for pending finalizers before shutting down MetaCall
	// call garbage collector twice to give enough finalizers time to execute
	runtime.GC()
	runtime.Gosched()
	runtime.GC()

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

// start localhost server for profiles for monitoring and debugging
// profiles can be accessed using command "go tool pprof http://localhost:6060/debug/pprof/name?debug=n"
// name is to be replaced with the wanted profile name and put n=0 for binary format, n>0 for plain text,
// n=2 for the full stack trace of all running goroutines in a format identical to an unrecovered panic.
// see https://jvns.ca/blog/2017/09/24/profiling-go-with-pprof/ for profiles names and more info about them
func startProfilesServer() {
	monitMux := http.NewServeMux()

	// register profiles
	monitMux.HandleFunc("/debug/pprof/", pprof.Index)          // heap and goroutine
	monitMux.HandleFunc("/debug/pprof/cmdline", pprof.Cmdline) // go command arguments
	monitMux.HandleFunc("/debug/pprof/symbol", pprof.Symbol)   // translation helper for memory addresses
	monitMux.HandleFunc("/debug/pprof/trace", pprof.Trace)     // trace

	// start goroutine for localhost to display the profiles
	go func() {
		err := http.ListenAndServe("localhost:6060", monitMux)
		if err != nil {
			return
		}
	}()
}
