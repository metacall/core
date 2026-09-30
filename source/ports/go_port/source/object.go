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

type Object struct {
	parentCls *Class
	val       unsafe.Pointer
	ptr       unsafe.Pointer
}

func newObject(value unsafe.Pointer, cls *Class) *Object {
	p := C.metacall_value_to_object(value)
	obj := &Object{parentCls: cls, val: value, ptr: p}
	// associate a finalizer so when value is not needed anymore GC destroy and free it
	runtime.SetFinalizer(obj, func(o *Object) {
		if o.val != nil {
			C.metacall_value_destroy(o.val)
			o.val = nil
			o.ptr = nil
			o.parentCls = nil
		}
	})
	return obj
}

func (o *Object) Get(key string) (interface{}, error) {
	if o.ptr == nil {
		return nil, errors.New("can't get attribute of nil object")
	}

	defer runtime.KeepAlive(o)

	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	ret := C.metacall_object_get(o.ptr, cKey)
	if ret == nil {
		return nil, errors.New("no attribute with this name: " + key)
	}

	id := C.metacall_value_id(ret)
	if id != C.METACALL_CLASS && id != C.METACALL_OBJECT && id != C.METACALL_FUTURE {
		defer C.metacall_value_destroy(ret)
	}

	val := valueToGo(ret)

	return val, nil
}

func (o *Object) Set(key string, val interface{}) error {
	if o.ptr == nil {
		return errors.New("can't set attribute of nil object")
	}

	defer runtime.KeepAlive(o)

	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var p unsafe.Pointer
	goToValue(val, &p)
	if p == nil {
		return errors.New("couldn't identify the value")
	}
	defer C.metacall_value_destroy(p)

	if C.metacall_object_set(o.ptr, cKey, p) != 0 {
		return errors.New("failed to set value to key " + key)
	}

	return nil
}

func (o *Object) Call(method string, args ...interface{}) (interface{}, error) {
	if o.ptr == nil {
		return nil, errors.New("can't call method of nil object")
	}

	defer runtime.KeepAlive(o)

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
	if ret == nil {
		return nil, errors.New("failed to call method: " + method)
	}

	id := C.metacall_value_id(ret)
	if id != C.METACALL_CLASS && id != C.METACALL_OBJECT && id != C.METACALL_FUTURE {
		defer C.metacall_value_destroy(ret)
	}

	val := valueToGo(ret)

	return val, nil
}
