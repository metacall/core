/*
 *	Memory Library by Parra Studios
 *	A generic cross-platform memory utility.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef MEMORY_ALLOCATOR_H
#define MEMORY_ALLOCATOR_H 1

/* -- Headers -- */

#include <memory/memory_api.h>

#include <memory/memory_allocator_iface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Forward Declarations -- */

struct memory_allocator_type;

/* -- Type Definitions -- */

typedef struct memory_allocator_type * memory_allocator;

/* -- Methods -- */

MEMORY_API memory_allocator memory_allocator_create(memory_allocator_iface iface, void * ctx);

MEMORY_API void * memory_allocator_allocate(memory_allocator allocator, size_t size);

MEMORY_API void * memory_allocator_reallocate(memory_allocator allocator, void * data, size_t size, size_t new_size);

MEMORY_API void memory_allocator_deallocate(memory_allocator allocator, void * data);

MEMORY_API size_t memory_allocator_used(memory_allocator allocator);

MEMORY_API void memory_allocator_destroy(memory_allocator allocator);

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_ALLOCATOR_H */
