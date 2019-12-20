package main

// #cgo CFLAGS: -Wall
// #cgo LDFLAGS: -lmetacall
// #include <metacall/metacall.h>
// #include <stdlib.h>
import "C"

import (
	"unsafe"
	"errors"
	"os"
	"fmt"
)

const PtrSizeInBytes = (32 << uintptr(^uintptr(0)>>63)) >> 3

func metacall_initialize() error {
	if (int(C.metacall_initialize()) != 0) {
		return errors.New("MetaCall failed to initialize")
	}

	return nil
}

func metacall_load_from_file(tag string, scripts []string) error {
	size := len(scripts)

	cTag := C.CString(tag)
	defer C.free(unsafe.Pointer(cTag))

	cScripts := C.malloc(C.size_t(size) * C.size_t(unsafe.Sizeof(uintptr(0))))
	defer C.free(unsafe.Pointer(cScripts))

	// Convert cScripts to a Go Array so we can index it
	goScripts := (*[1 << 30 - 1] * C.char)(cScripts)

	for index, script := range scripts {
		goScripts[index] = C.CString(script)
	}

	if int(C.metacall_load_from_file(cTag, (**C.char)(cScripts), (C.size_t)(size), nil)) != 0 {
		return errors.New("MetaCall failed to load script")
	}

	return nil
}

func metacall(function string, args ...interface{}) (interface{}, error) {

	cFunction := C.CString(function)
	defer C.free(unsafe.Pointer(cFunction))

	cFunc := C.metacall_function(cFunction)

	if cFunc == nil {
		return nil, errors.New("Function not found")
	}

	size := len(args)

	cArgs := C.malloc(C.size_t(size) * C.size_t(unsafe.Sizeof(uintptr(0))))
	defer C.free(unsafe.Pointer(cArgs))

	for index, arg := range args {
		cArg := (*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(cArgs)) + uintptr(index) * PtrSizeInBytes))

		// Create int
		if i, ok := arg.(int); ok {
			*cArg = C.metacall_value_create_int((C.int)(i))
		}

		// Create string
		if str, ok := arg.(string); ok {
			cStr := C.CString(str)
			defer C.free(unsafe.Pointer(cStr))
			*cArg = C.metacall_value_create_string(cStr, (C.size_t)(len(str)))
		}

		// TODO: Other types ...
	}

	defer (func () {
		for index, _ := range args {
			cArg := (*unsafe.Pointer)(unsafe.Pointer(uintptr(unsafe.Pointer(cArgs)) + uintptr(index) * PtrSizeInBytes))
			C.metacall_value_destroy(*cArg)
		}
	})()

	ret := C.metacallfv(cFunc, (*unsafe.Pointer)(cArgs))

	if ret != nil {
		defer C.metacall_value_destroy(ret)

		switch (C.metacall_value_id(unsafe.Pointer(ret))) {
			case C.METACALL_INT: {
				return int(C.metacall_value_to_int(unsafe.Pointer(ret))), nil
			}
			case C.METACALL_STRING: {
				return C.GoString(C.metacall_value_to_string(unsafe.Pointer(ret))), nil
			}
		}
	}

	return nil, nil
}

func metacall_destroy() {
	C.metacall_destroy()
}

func main() {

	if err := metacall_initialize(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	defer metacall_destroy()

	scripts := []string{ "test.mock" }

	if err := metacall_load_from_file("mock", scripts); err != nil {
		fmt.Println(err)
		return
	}

	ret, err := metacall("three_str", "e", "f", "g")

	if err != nil {
		fmt.Println(err)
		return
	}

	if str, ok := ret.(string); ok {
		fmt.Println(str)
	}
}
