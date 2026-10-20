package metacall

/*
#cgo CFLAGS: -Wall
#cgo LDFLAGS: -lmetacall

#include <metacall/metacall.h>
*/
import "C"

import (
	"errors"
	"runtime"
	"unsafe"
)

type Class struct {
	reg uint64 // Handle of the value in the registry
	val unsafe.Pointer
	ptr unsafe.Pointer
}

func newClass(value unsafe.Pointer) *Class {
	value = C.metacall_value_copy(value)
	p := C.metacall_value_to_class(value)
	cls := &Class{reg: registry.save(value), val: value, ptr: p}
	// Associate a finalizer so when value is not needed anymore GC destroy and free it
	runtime.SetFinalizer(cls, func(c *Class) {
		if c.val != nil {
			if v := registry.delete(c.reg); v != nil {
				C.metacall_value_destroy(v)
			}
			c.val = nil
			c.ptr = nil
		}
	})
	return cls
}

func (c *Class) New(name string, args ...interface{}) (*Object, error) {
	if c.ptr == nil {
		return nil, errors.New("can't use nil ptr for class creation")
	}

	cName := C.CString(name)
	defer C.free(unsafe.Pointer(cName))

	argNum := C.size_t(len(args))
	var cArgs unsafe.Pointer

	if argNum > 0 {
		cArgs = C.malloc(argNum * C.size_t(unsafe.Sizeof(uintptr(0))))

		defer func() {
			for index := range args {
				argPtr := *(*unsafe.Pointer)(unsafe.Pointer(uintptr(cArgs) + uintptr(index)*PtrSizeInBytes))

				if argPtr != nil {
					C.metacall_value_destroy(argPtr)
				}
			}
			C.free(cArgs)
		}()
	}

	for idx, arg := range args {
		goToValue(arg, (*unsafe.Pointer)(unsafe.Pointer(uintptr(cArgs)+uintptr(idx)*PtrSizeInBytes)))
	}

	cls := C.metacall_class_new(c.ptr, cName, (*unsafe.Pointer)(cArgs), argNum)

	// Keep the class alive until the call finishes, otherwise its finalizer can destroy the value meanwhile
	runtime.KeepAlive(c)

	if cls == nil {
		return nil, errors.New("failed to create class: " + name)
	}

	defer C.metacall_value_destroy(cls)

	obj := newObject(cls, c)

	return obj, nil
}

func (c *Class) StaticGet(key string) (interface{}, error) {
	if c.ptr == nil {
		return nil, errors.New("can't get attribute of nil class")
	}

	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	ret := C.metacall_class_static_get(c.ptr, cKey)

	// Keep the class alive until the call finishes, otherwise its finalizer can destroy the value meanwhile
	runtime.KeepAlive(c)

	if ret == nil {
		return nil, errors.New("no attribute with this name: " + key)
	}

	val := valueToGoOwned(ret)

	return val, nil
}

func (c *Class) StaticSet(key string, val interface{}) error {
	if c.ptr == nil {
		return errors.New("can't set attribute of nil class")
	}

	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var p unsafe.Pointer
	goToValue(val, &p)
	if p == nil {
		return errors.New("couldn't identify the value")
	}
	defer C.metacall_value_destroy(p)

	result := C.metacall_class_static_set(c.ptr, cKey, p)

	// Keep the class alive until the call finishes, otherwise its finalizer can destroy the value meanwhile
	runtime.KeepAlive(c)

	if result != 0 {
		return errors.New("failed to set value to key " + key)
	}

	return nil
}
