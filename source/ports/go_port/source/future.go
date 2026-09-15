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
	"unsafe"
)

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
	ctxPtr := pointerSave(callbacks)

	C.metacall_await_future(f.ptr, C.metacall_await_callback(C.resolveCgo), C.metacall_await_callback(C.rejectCgo), ctxPtr)

	// block until a value returned
	ret := <-retValCh
	if ret.value == nil && ret.err == nil {
		pointerDelete(ctxPtr)
		return nil, errors.New("failed to get value from future")
	}

	return ret.value, ret.err
}
