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

type Object struct {
	reg uint64 // Handle of the value in the registry
	cls *Class
	val unsafe.Pointer
	ptr unsafe.Pointer
}

func newObject(value unsafe.Pointer, cls *Class) *Object {
	value = C.metacall_value_copy(value)
	p := C.metacall_value_to_object(value)
	obj := &Object{reg: registry.save(value), cls: cls, val: value, ptr: p}
	// Associate a finalizer so when value is not needed anymore GC destroy and free it
	runtime.SetFinalizer(obj, func(o *Object) {
		if o.val != nil {
			if v := registry.delete(o.reg); v != nil {
				C.metacall_value_destroy(v)
			}
			o.val = nil
			o.ptr = nil
			o.cls = nil
		}
	})
	return obj
}

func (o *Object) Get(key string) (interface{}, error) {
	if o.ptr == nil {
		return nil, errors.New("can't get attribute of nil object")
	}

	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	ret := C.metacall_object_get(o.ptr, cKey)
	// Keep the wrapper alive until the call finishes, otherwise its finalizer can destroy the value meanwhile
	runtime.KeepAlive(o)

	if ret == nil {
		return nil, errors.New("no attribute with this name: " + key)
	}

	val := valueToGoOwned(ret)

	return val, nil
}

func (o *Object) Set(key string, val interface{}) error {
	if o.ptr == nil {
		return errors.New("can't set attribute of nil object")
	}

	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var p unsafe.Pointer
	goToValue(val, &p)
	if p == nil {
		return errors.New("couldn't identify the value")
	}
	defer C.metacall_value_destroy(p)

	result := C.metacall_object_set(o.ptr, cKey, p)
	// Keep the wrapper alive until the call finishes, otherwise its finalizer can destroy the value meanwhile
	runtime.KeepAlive(o)

	if result != 0 {
		return errors.New("failed to set value to key " + key)
	}

	return nil
}

func (o *Object) Call(method string, args ...interface{}) (interface{}, error) {
	if o.ptr == nil {
		return nil, errors.New("can't call method of nil object")
	}

	cMethod := C.CString(method)
	defer C.free(unsafe.Pointer(cMethod))

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

	ret := C.metacallv_object(o.ptr, cMethod, (*unsafe.Pointer)(cArgs), argNum)
	// Keep the wrapper alive until the call finishes, otherwise its finalizer can destroy the value meanwhile
	runtime.KeepAlive(o)

	if ret == nil {
		return nil, errors.New("failed to call method: " + method)
	}

	val := valueToGoOwned(ret)

	return val, nil
}
