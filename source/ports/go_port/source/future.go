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

type Future struct {
	reg uint64 // Handle of the value in the registry
	val unsafe.Pointer
	ptr unsafe.Pointer
}

type awaitFutureSafeWork struct {
	future  *Future
	ret     chan callReturnSafeWork
	resolve awaitCallback
	reject  awaitCallback
	ctx     interface{}
}

func (w *awaitFutureSafeWork) execute() {
	value, err := w.future.AwaitUnsafe(w.resolve, w.reject, w.ctx)
	w.ret <- callReturnSafeWork{value, err}
}
func (w *awaitFutureSafeWork) cancel(err error) {
	w.ret <- callReturnSafeWork{nil, err}
}

func newFuture(value unsafe.Pointer) *Future {
	value = C.metacall_value_copy(value)
	p := C.metacall_value_to_future(value)
	fut := &Future{reg: registry.save(value), val: value, ptr: p}
	runtime.SetFinalizer(fut, func(f *Future) {
		if f.val != nil {
			if v := registry.delete(f.reg); v != nil {
				C.metacall_value_destroy(v)
			}
			f.ptr = nil
			f.val = nil
		}
	})
	return fut
}

func (f *Future) AwaitUnsafe(resolve, reject awaitCallback, ctx interface{}) (interface{}, error) {
	if f == nil || f.ptr == nil {
		return nil, errors.New("can't await a nil future")
	}

	cCallbacks, goCallbacksPtr := awaitCallbacksSave(resolve, reject, ctx)

	ret := C.metacall_await_future(f.ptr, cCallbacks.resolve, cCallbacks.reject, goCallbacksPtr)

	// Keep f alive until the await has been registered to prevent early finalizer destruction
	runtime.KeepAlive(f)

	if ret != nil {
		v := valueToGoOwned(ret)
		if err, ok := v.(error); ok {
			return nil, err
		}
		return v, nil
	} else {
		// Delete and free ptr if metacall_await_future failed with nil
		pointerDelete(goCallbacksPtr)
	}

	return nil, nil
}

// Await sends the asynchronous work for the future and blocks until it's processed (it does not wait for the future to be resolved or rejected)
func (f *Future) Await(resolve, reject awaitCallback, ctx interface{}) (interface{}, error) {
	rCtx := checkRootCtx()
	if rCtx == nil {
		return nil, errShutdown
	}

	ret := make(chan callReturnSafeWork, 1)

	w := &awaitFutureSafeWork{
		future:  f,
		ret:     ret,
		resolve: resolve,
		reject:  reject,
		ctx:     ctx,
	}

	select {
	case <-rCtx.Done():
		return nil, errShutdown
	case queue <- w:
	}

	select {
	case <-rCtx.Done():
		return nil, errShutdown
	case result := <-ret:
		return result.value, result.err
	}
}
