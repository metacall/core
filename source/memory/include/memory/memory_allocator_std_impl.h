/*
 *	Memory Library by Parra Studios
 *	A generic cross-platform memory utility.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef MEMORY_ALLOCATOR_STD_IMPL_H
#define MEMORY_ALLOCATOR_STD_IMPL_H 1

/* -- Headers -- */

#include <memory/memory_api.h>

#include <memory/memory_allocator_iface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

	struct memory_allocator_std_ctx_type;

/* -- Type Definitions -- */

typedef void * (*memory_allocator_std_impl_malloc)(size_t);

typedef void * (*memory_allocator_std_impl_realloc)(void *, size_t);

typedef void (*memory_allocator_std_impl_free)(void *);

typedef struct memory_allocator_std_ctx_type * memory_allocator_std_ctx;

/* -- Member Data -- */

struct memory_allocator_std_ctx_type
{
	memory_allocator_std_impl_malloc malloc;
	memory_allocator_std_impl_realloc realloc;
	memory_allocator_std_impl_free free;
};

/* -- Methods -- */

MEMORY_API memory_allocator_iface memory_allocator_std_iface(void);

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_ALLOCATOR_STD_IMPL_H */
