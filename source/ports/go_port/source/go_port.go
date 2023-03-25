/*
 *	MetaCall Go Port by Parra Studios
 *	A frontend for Go language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
	"errors"
	"fmt"
	"runtime"
	"sync"
	"unsafe"
)

type loadFromFileSafeWork struct {
	tag     string
	scripts []string
	err     chan error
}

type loadFromMemorySafeWork struct {
	tag    string
	buffer string
	err    chan error
}

type callReturnSafeWork struct {
	value interface{}
	err   error
}

type callSafeWork struct {
	function string
	args     []interface{}
	ret      chan callReturnSafeWork
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

type awaitCallbacks struct {
	resolve awaitCallback
	reject  awaitCallback
	ctx     interface{}
}

const PtrSizeInBytes = (32 << (^uintptr(0) >> 63)) >> 3

var (
	queue  = make(chan interface{}, 1) // Queue for dispatching the work
	toggle chan struct{}               // Channel for stopping the queue
	lock   sync.Mutex                  // Lock for the queue
	wg     sync.WaitGroup              // Wait group for the queue (needed to obtain return values)
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
	lock.Lock()
	defer lock.Unlock()

	if toggle != nil {
		// Already running
		return nil
	}

	toggle = make(chan struct{}, 1)
	initErr := make(chan error, 1)

	go func(initErr chan error, toggle <-chan struct{}) {
		// Bind this goroutine to its thread
		runtime.LockOSThread()

		// Initialize MetaCall
		if err := InitializeUnsafe(); err != nil {
			initErr <- err
			return
		}

		close(initErr)

		for {
			select {
			case <-toggle:
				// Shutdown
				DestroyUnsafe()
				return
			case w := <-queue:
				switch v := w.(type) {
				case loadFromFileSafeWork:
					err := LoadFromFileUnsafe(v.tag, v.scripts)
					v.err <- err
				case loadFromMemorySafeWork:
					err := LoadFromMemoryUnsafe(v.tag, v.buffer)
					v.err <- err
				case callSafeWork:
					value, err := CallUnsafe(v.function, v.args...)
					v.ret <- callReturnSafeWork{value, err}
				case awaitSafeWork:
					value, err := AwaitUnsafe(v.function, v.resolve, v.reject, v.ctx, v.args...)
					v.ret <- callReturnSafeWork{value, err}
				}
				wg.Done()
			}
		}
	}(initErr, toggle)

	return <-initErr
}

func LoadFromFileUnsafe(tag string, scripts []string) error {
	size := len(scripts)

	cTag := C.CString(tag)
	defer C.free(unsafe.Pointer(cTag))

	cScripts := C.malloc(C.size_t(size) * C.size_t(unsafe.Sizeof(uintptr(0))))
	defer C.free(unsafe.Pointer(cScripts))

	// Convert cScripts to a Go Array so we can index it
	goScripts := (*[1<<30 - 1]*C.char)(cScripts)

	for index, script := range scripts {
		goScripts[index] = C.CString(script)
	}

	if int(C.metacall_load_from_file(cTag, (**C.char)(cScripts), (C.size_t)(size), nil)) != 0 {
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
		for index := range args {
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
	ret := make(chan callReturnSafeWork, 1)

	w := callSafeWork{
		function: function,
		args:     args,
		ret:      ret,
	}

	wg.Add(1)
	queue <- w

	result := <-ret

	return result.value, result.err
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
		for index := range args {
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
	}

	return nil, nil
}

// Await sends asynchronous work and blocks until it's processed
func Await(function string, resolve, reject awaitCallback, ctx interface{}, args ...interface{}) (interface{}, error) {
	ret := make(chan callReturnSafeWork, 1)

	w := awaitSafeWork{
		function: function,
		args:     args,
		ret:      ret,
		resolve:  resolve,
		reject:   reject,
		ctx:      ctx,
	}

	wg.Add(1)
	queue <- w

	result := <-ret

	return result.value, result.err
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
	switch v := arg.(type) {
	case bool:
		if v {
			*ptr = C.metacall_value_create_bool(C.int(1))
		} else {
			*ptr = C.metacall_value_create_bool(C.int(0))
		}
	case int:
		*ptr = C.metacall_value_create_int(C.int(v))
	case int16:
		*ptr = C.metacall_value_create_short(C.short(v))
	case int32:
		*ptr = C.metacall_value_create_long(C.long(v))
	case float32:
		*ptr = C.metacall_value_create_float(C.float(v))
	case float64:
		*ptr = C.metacall_value_create_double(C.double(v))
	case byte:
		*ptr = C.metacall_value_create_char(C.char(v))
	case string:
		cStr := C.CString(v)
		defer C.free(unsafe.Pointer(cStr))
		*ptr = C.metacall_value_create_string(cStr, (C.size_t)(len(v)))
	}
	// TODO: Add more types
}

func valueToGo(value unsafe.Pointer) interface{} {
	switch C.metacall_value_id(value) {
	case C.METACALL_BOOL:
		return int(C.metacall_value_to_bool(value)) != 0
	case C.METACALL_INT:
		return int(C.metacall_value_to_int(value))
	case C.METACALL_SHORT:
		return int16(C.metacall_value_to_short(value))
	case C.METACALL_LONG:
		return int32(C.metacall_value_to_long(value))
	case C.METACALL_FLOAT:
		return float32(C.metacall_value_to_float(value))
	case C.METACALL_DOUBLE:
		return float64(C.metacall_value_to_double(value))
	case C.METACALL_CHAR:
		return byte(C.metacall_value_to_char(value))
	case C.METACALL_STRING:
		return C.GoString(C.metacall_value_to_string(value))
	case C.METACALL_ARRAY:
		arrayValue := C.metacall_value_to_array(value)
		arraySize := C.metacall_value_count(value)
		array := make([]interface{}, arraySize)

		for iterator := C.size_t(0); iterator < arraySize; iterator++ {
			currentValue := (*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(arrayValue)) + uintptr(iterator*PtrSizeInBytes)))
			array[iterator] = valueToGo(*currentValue)
		}

		return array
		// TODO: Add more types
	}
	return nil
}

func LoadFromFile(tag string, scripts []string) error {
	result := make(chan error, 1)
	w := loadFromFileSafeWork{
		tag,
		scripts,
		result,
	}
	wg.Add(1)
	queue <- w

	return <-result
}

func LoadFromMemory(tag string, buffer string) error {
	result := make(chan error, 1)
	w := loadFromMemorySafeWork{
		tag,
		buffer,
		result,
	}
	wg.Add(1)
	queue <- w

	return <-result
}

func DestroyUnsafe() {
	C.metacall_destroy()
}

// Shutdown disables the metacall adapter waiting for all calls to complete
func Destroy() {
	lock.Lock()
	close(toggle)
	toggle = nil
	lock.Unlock()

	// Wait for all work to complete
	wg.Wait()
}
