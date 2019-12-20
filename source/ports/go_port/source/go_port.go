package main

// #cgo CFLAGS: -Wall
// #cgo LDFLAGS: -lmetacall -lpthread
// #include <metacall/metacall.h>
// #include <stdlib.h>
import "C"

import (
	"unsafe"
	"os"
	"fmt"
)

func metacall_initialize() int {
	return int(C.metacall_initialize())
}

func metacall_load_from_file(tag string, scripts []string) int {
	length := len(scripts)

	cTag := C.CString(tag)
	defer C.free(unsafe.Pointer(cTag))

	cScripts := C.malloc(C.size_t(length) * C.size_t(unsafe.Sizeof(uintptr(0))))
	defer C.free(unsafe.Pointer(cScripts))

	// Convert cScripts to a Go Array so we can index it
	goScripts := (*[1 << 30 - 1] * C.char)(cScripts)

	for index, script := range scripts {
		goScripts[index] = C.CString(script)
	}

	return int(C.metacall_load_from_file(cTag, (**C.char)(cScripts), (C.size_t)(length), nil))
}

func metacall(function string, args ...interface{}) interface{} {
	// TODO
	return nil
}

func metacall_destroy() {
	C.metacall_destroy()
}

func main() {
	if (metacall_initialize() != 0) {
		os.Exit(1)
	}

	defer metacall_destroy()

	scripts := []string{ "test.mock" }

	if (metacall_load_from_file("mock", scripts) != 0) {

		result := metacall("three_str", "a", "b", "c")

		fmt.Println(result.(string))
	}
}
