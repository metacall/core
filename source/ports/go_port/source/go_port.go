package metacall

// #cgo CFLAGS: -Wall
// #cgo LDFLAGS: -lmetacall
// #include <metacall/metacall.h>
// #include <stdlib.h>
import "C"

import (
	"fmt"
	"runtime"
	"sync"
	"unsafe"
	"errors"
)

type loadFromFileSafeWork struct {
	tag     string
	scripts []string
	err     chan error
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

type callSafeAsyncWork struct {
	function string
	args     []interface{}
	ret      chan callReturnSafeWork
	resolve	 func(unsafe.Pointer, unsafe.Pointer)
	reject	 func(unsafe.Pointer, unsafe.Pointer)
}

const PtrSizeInBytes = (32 << uintptr(^uintptr(0)>>63)) >> 3

var queue = make(chan interface{}, 1)
var toggle chan struct{}
var lock sync.Mutex
var wg sync.WaitGroup

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
					{
						err := LoadFromFileUnsafe(v.tag, v.scripts)
						v.err <- err
					}
				case callSafeWork:
					{
						value, err := CallUnsafe(v.function, v.args...)
						v.ret <- callReturnSafeWork{value, err}
					}
				case callSafeAsyncWork:
					{
						value, err := CallAwaitUnsafe(v.function, v.resolve, v.reject, v.args...)
						v.ret <- callReturnSafeWork{value, err}
					}
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

func CallAwaitUnsafe(function string, ResolveCallback, RejectCallBack func(unsafe.Pointer, unsafe.Pointer), args ...interface{}) (interface{}, error) {
	cFunc, err := StringToCFunc(function)
	if err != nil {
		return nil, err
	}

	cArgs := C.malloc(C.size_t(len(args)) * C.size_t(unsafe.Sizeof(uintptr(0))))
	for index, arg := range args {
		GoArgToCAPI(arg, (*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(cArgs)) + uintptr(index)*PtrSizeInBytes)))
	}
	defer C.free(unsafe.Pointer(cArgs))
	defer func(){
		for index, _ := range args {
			C.metacall_value_destroy(*(*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(cArgs)) + uintptr(index)*PtrSizeInBytes)))
		}
	}()

	var Context interface{} // undefined

	ret := C.metacallfv_await(cFunc, (*unsafe.Pointer)(cArgs), unsafe.Pointer(&ResolveCallback), unsafe.Pointer(&RejectCallBack), unsafe.Pointer(&Context))

	if ret != nil {
		defer C.metacall_value_destroy(ret)
		return CAPIValueToGo(ret), nil
	}

	return nil, nil
}

func CallUnsafe(function string, args ...interface{}) (interface{}, error) {
	cFunc, err := StringToCFunc(function)
	if err != nil {
		return nil, err
	}

	cArgs := C.malloc(C.size_t(len(args)) * C.size_t(unsafe.Sizeof(uintptr(0))))
	for index, arg := range args {
		GoArgToCAPI(arg, (*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(cArgs)) + uintptr(index)*PtrSizeInBytes)))
	}
	defer C.free(unsafe.Pointer(cArgs))
	defer func(){
		for index, _ := range args {
			C.metacall_value_destroy(*(*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(cArgs)) + uintptr(index)*PtrSizeInBytes)))
		}
	}()

	ret := C.metacallfv(cFunc, (*unsafe.Pointer)(cArgs))

	if ret != nil {
		defer C.metacall_value_destroy(ret)
		return CAPIValueToGo(ret), nil
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

// Await sends asynchronous work and blocks until it's processed
func Await(function string, args ...interface{}) (interface{}, error) {
	ret := make(chan callReturnSafeWork, 1)

	Resolve := func(pointer unsafe.Pointer, pointer2 unsafe.Pointer) {
		// todo

	}

	Reject := func(pointer unsafe.Pointer, pointer2 unsafe.Pointer) {
		// todo

	}

	w := callSafeAsyncWork{
		function: function,
		args:     args,
		ret:      ret,
		resolve: Resolve,
		reject: Reject,
	}

	wg.Add(1)
	queue <- w

	result := <-ret

	return result.value, result.err
}

func StringToCFunc(FString string) (unsafe.Pointer, error) {
	cFunction := C.CString(FString)
	defer C.free(unsafe.Pointer(cFunction))
	cFunc := C.metacall_function(cFunction)
	if cFunc == nil {
		return nil, errors.New("function %s not found: " + FString)
	}
	return cFunc, nil
}

func GoArgToCAPI(Arg interface{}, Ptr *unsafe.Pointer) {
	// Create int
	if i, ok := Arg.(int); ok {
		*Ptr = C.metacall_value_create_int((C.int)(i))
	}

	// Create float32
	if i, ok := Arg.(float32); ok {
		*Ptr = C.metacall_value_create_float((C.float)(i))
	}

	// Create float64
	if i, ok := Arg.(float64); ok {
		*Ptr = C.metacall_value_create_double((C.double)(i))
	}

	// Create string
	if str, ok := Arg.(string); ok {
		cStr := C.CString(str)
		defer C.free(unsafe.Pointer(cStr))
		*Ptr = C.metacall_value_create_string(cStr, (C.size_t)(len(str)))
	}
}

func CAPIValueToGo(value unsafe.Pointer) interface{} {
	switch C.metacall_value_id(value) {
	case C.METACALL_INT:
		{
			return int(C.metacall_value_to_int(value))
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

		// ...
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
