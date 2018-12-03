/*
 *	Memory Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic cross-platform memory utility.
 *
 */

#ifndef MEMORY_ALLOCATOR_STD_H
#define MEMORY_ALLOCATOR_STD_H 1

/* -- Headers -- */

#include <memory/memory_api.h>

#include <memory/memory_allocator.h>
#include <memory/memory_allocator_std_impl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

MEMORY_API memory_allocator memory_allocator_std(memory_allocator_std_impl_malloc malloc, memory_allocator_std_impl_realloc realloc, memory_allocator_std_impl_free free);

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_ALLOCATOR_STD_H */
