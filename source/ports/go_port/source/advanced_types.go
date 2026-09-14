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
	"fmt"
	"runtime"
	"runtime/cgo"
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

	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	ret := C.metacall_object_get(o.ptr, cKey)
	if ret == nil {
		return nil, errors.New("no attribute with this name: " + key)
	}

	val := valueToGo(ret)

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

	if C.metacall_object_set(o.ptr, cKey, p) != 0 {
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
	if ret == nil {
		return nil, errors.New("failed to call method: " + method)
	}

	val := valueToGo(ret)

	return val, nil
}

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

type Future struct {
	val unsafe.Pointer
	ptr unsafe.Pointer
}
type futureResult struct {
	value interface{}
	err   error
}

func newFuture(value unsafe.Pointer) *Future {
	p := C.metacall_value_to_future(value)
	fut := &Future{val: value, ptr: p}
	runtime.SetFinalizer(fut, func(f *Future) {
		if f != nil {
			C.metacall_value_destroy(f.val)
			f.ptr = nil
			f.val = nil
		}
	})
	return fut
}

func (f *Future) Await() (interface{}, error) {
	if f.ptr == nil {
		return nil, errors.New("can't call async on nil future")
	}

	// destroy the handle when future complete
	defer func() {
		if f.val != nil {
			C.metacall_value_destroy(f.val)
			f.ptr = nil
			f.val = nil
		}
	}()

	retValCh := make(chan futureResult, 1)

	//create callbacks for metacall_await_future to use them via goReject & goResolve
	callbacks := &awaitCallbacks{
		resolve: func(val, _ interface{}) interface{} {
			retValCh <- futureResult{value: val, err: nil}
			return nil
		},
		reject: func(val, _ interface{}) interface{} {
			if e, ok := val.(error); ok {
				retValCh <- futureResult{value: nil, err: e}
			} else {
				retValCh <- futureResult{value: nil, err: fmt.Errorf("%v", val)}
			}
			return nil
		},
	}

	// use handle to prevent breaking cgo pointer-passing rules
	// await.go calls goResolve or goReject convert ctxPtr back and reads its content
	handle := cgo.NewHandle(callbacks)
	ctxPtr := unsafe.Pointer(uintptr(handle))

	C.metacall_await_future(f.ptr, C.metacall_await_callback(C.resolveCgo), C.metacall_await_callback(C.rejectCgo), ctxPtr)

	// block until a value returned
	ret := <-retValCh
	if ret.value == nil && ret.err == nil {
		return nil, errors.New("failed to get value from future")
	}

	return ret.value, ret.err
}
