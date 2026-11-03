/*
 *	MetaCall Go Port by Parra Studios
 *	A frontend for Go language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

import (
	"sync/atomic"
	"unsafe"
)

// Implements a lock free registry for the values owned by the wrappers (class, object and future), the finalizers
// are not guaranteed to run, so the values still registered are destroyed when MetaCall is destroyed (drain)
//
// The values are stored in slots allocated by chunks, the chunks are never moved or freed, so a slot can be accessed
// without locks. Each slot has a tag that is incremented each time the slot is reused, the handle returned to the
// wrappers contains the index and the tag of the slot, so a finalizer that runs after the drain cannot release the
// value of a new wrapper that reuses the same slot. The value is released by the one (finalizer or drain) that clears
// the live bit of the slot with a CAS, and then the slot is pushed into a lock free stack (free list) for being reused
//
// The values must not be registered while draining, which is guaranteed because the drain is done in the MetaCall
// thread after canceling the queue of calls, and the wrappers are only created from the results of the calls

const (
	registryChunkBits = 12
	registryChunkSize = 1 << registryChunkBits             // Number of slots of a chunk
	registryChunkMask = registryChunkSize - 1              // Mask for obtaining the position of a slot in its chunk
	registryChunks    = 1 << 12                            // Maximum number of chunks
	registrySize      = registryChunkSize * registryChunks // Maximum number of values registered at the same time
)

type registrySlot struct {
	state uint64         // Tag of the slot (shifted one bit) and live bit, it must be the first field for being 64-bit aligned
	value unsafe.Pointer // Value owned by the wrapper, it is published and claimed through the state
	next  uint32         // Next slot in the free list (index + 1, zero means end of the list)
}

type registryChunk [registryChunkSize]registrySlot

type valueRegistry struct {
	free  uint64                         // Head of the free list, counter for avoiding ABA (upper 32 bits) and index + 1 (lower 32 bits), it must be the first field for being 64-bit aligned
	top   uint32                         // Number of slots that have been used at least once
	table [registryChunks]unsafe.Pointer // Chunks of slots, allocated on demand
}

// Registry of the values owned by the wrappers (class, object and future)
var registry valueRegistry

// Get a slot whose chunk has been already allocated
func (r *valueRegistry) slot(index uint32) *registrySlot {
	chunk := (*registryChunk)(atomic.LoadPointer(&r.table[index>>registryChunkBits]))
	return &chunk[index&registryChunkMask]
}

// Allocate the chunk of a slot if it does not exist, if many goroutines allocate it at the same time only one is stored
func (r *valueRegistry) alloc(index uint32) {
	ptr := &r.table[index>>registryChunkBits]

	if atomic.LoadPointer(ptr) == nil {
		atomic.CompareAndSwapPointer(ptr, nil, unsafe.Pointer(new(registryChunk)))
	}
}

// Pop a slot from the free list
func (r *valueRegistry) pop() (uint32, bool) {
	for {
		head := atomic.LoadUint64(&r.free)
		index := uint32(head)

		if index == 0 {
			return 0, false
		}

		// The slot can be popped and reused meanwhile, then next is not valid but the CAS fails because the counter changed
		next := atomic.LoadUint32(&r.slot(index - 1).next)

		if atomic.CompareAndSwapUint64(&r.free, head, ((head>>32)+1)<<32|uint64(next)) {
			return index - 1, true
		}
	}
}

// Push a slot into the free list
func (r *valueRegistry) push(index uint32) {
	slot := r.slot(index)

	for {
		head := atomic.LoadUint64(&r.free)
		atomic.StoreUint32(&slot.next, uint32(head))

		if atomic.CompareAndSwapUint64(&r.free, head, ((head>>32)+1)<<32|uint64(index+1)) {
			return
		}
	}
}

// Release the value of a slot if its state is still the expected one, only one of the callers obtains the value
func (r *valueRegistry) claim(index uint32, state uint64) unsafe.Pointer {
	slot := r.slot(index)

	if !atomic.CompareAndSwapUint64(&slot.state, state, state&^1) {
		return nil
	}

	// The value is read after the CAS, which happens after the store of the state that published it
	value := slot.value
	slot.value = nil

	r.push(index)

	return value
}

// Register the value of a wrapper, returns the handle that must be used for deleting it (zero is never a valid handle)
func (r *valueRegistry) save(value unsafe.Pointer) uint64 {
	if value == nil {
		return 0
	}

	index, ok := r.pop()

	if !ok {
		index = atomic.AddUint32(&r.top, 1) - 1

		if index >= registrySize {
			panic("metacall: registry of wrappers is full, too many classes, objects or futures alive at the same time")
		}

		r.alloc(index)
	}

	slot := r.slot(index)

	// The tag starts at one, so zero is never a valid handle
	tag := uint32(atomic.LoadUint64(&slot.state)>>1) + 1

	if tag == 0 {
		tag = 1
	}

	// The value is written before the store of the state, so it is visible to the one that claims the slot
	slot.value = value
	atomic.StoreUint64(&slot.state, uint64(tag)<<1|1)

	return uint64(tag)<<32 | uint64(index)
}

// Unregister the value of a wrapper, returns the value if it has to be destroyed by the caller or nil if it has been already released
func (r *valueRegistry) delete(handle uint64) unsafe.Pointer {
	if handle == 0 {
		return nil
	}

	return r.claim(uint32(handle), (handle>>32)<<1|1)
}

// Unregister all the values and destroy them, it is used when MetaCall is destroyed
func (r *valueRegistry) drain(destroy func(value unsafe.Pointer)) {
	top := atomic.LoadUint32(&r.top)

	if top > registrySize {
		top = registrySize
	}

	for index := uint32(0); index < top; index++ {
		chunk := (*registryChunk)(atomic.LoadPointer(&r.table[index>>registryChunkBits]))

		if chunk == nil {
			// The chunk is being allocated, its slots have not been registered yet, so skip it
			index |= registryChunkMask
			continue
		}

		state := atomic.LoadUint64(&chunk[index&registryChunkMask].state)

		if state&1 == 0 {
			continue
		}

		if value := r.claim(index, state); value != nil {
			destroy(value)
		}
	}
}
