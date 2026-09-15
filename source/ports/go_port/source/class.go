package metacall

/*
#cgo CFLAGS: -Wall
#cgo LDFLAGS: -lmetacall

#include <metacall/metacall.h>

extern void *resolveCgo(void *, void *);
extern void *rejectCgo(void *, void *);
*/
import "C"

import (
	"errors"
	"runtime"
	"unsafe"
)

type Class struct {
	val unsafe.Pointer
	ptr unsafe.Pointer
}

func newClass(value unsafe.Pointer) *Class {
	p := C.metacall_value_to_class(value)
	cls := &Class{val: value, ptr: p}
	// associate a finalizer so when value is not needed anymore GC destroy and free it
	runtime.SetFinalizer(cls, func(c *Class) {
		if c.val != nil {
			C.metacall_value_destroy(c.val)
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

	if cls == nil {
		return nil, errors.New("failed to create class: " + name)
	}

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

	if ret == nil {
		return nil, errors.New("no attribute with this name: " + key)
	}

	val := valueToGo(ret)

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

	if C.metacall_class_static_set(c.ptr, cKey, p) != 0 {
		return errors.New("failed to set value to key " + key)
	}

	return nil
}
