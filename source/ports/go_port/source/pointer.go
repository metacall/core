/*
 *	MetaCall Go Port by Parra Studios
 *	A frontend for Go language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

package metacall

// #include <stdlib.h>
import "C"

import (
	"sync"
	"unsafe"
)

// Implements a way to store Go data and make it accessible from C
// Based on: https://github.com/mattn/go-pointer
var (
	mutex sync.RWMutex
	store = map[unsafe.Pointer]interface{}{}
)

func pointerSave(v interface{}) unsafe.Pointer {
	if v == nil {
		return nil
	}

	var ptr unsafe.Pointer = C.malloc(C.size_t(1))

	if ptr == nil {
		return nil
	}

	mutex.Lock()
	store[ptr] = v
	mutex.Unlock()

	return ptr
}

func pointerGet(ptr unsafe.Pointer) interface{} {
	if ptr == nil {
		return nil
	}

	mutex.RLock()
	v := store[ptr]
	mutex.RUnlock()

	return v
}

func pointerDelete(ptr unsafe.Pointer) {
	if ptr == nil {
		return
	}

	mutex.Lock()
	delete(store, ptr)
	mutex.Unlock()

	C.free(ptr)
}
