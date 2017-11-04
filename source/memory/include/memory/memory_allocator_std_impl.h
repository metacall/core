/*
 *	Memory Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic cross-platform memory utility.
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
