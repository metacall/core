/*
 *	Memory Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic cross-platform memory utility.
 *
 */

#ifndef MEMORY_ALLOCATOR_NGINX_H
#define MEMORY_ALLOCATOR_NGINX_H 1

/* -- Headers -- */

#include <memory/memory_api.h>

#include <memory/memory_allocator.h>
#include <memory/memory_allocator_nginx_impl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

MEMORY_API memory_allocator memory_allocator_nginx(void * pool, memory_allocator_nginx_impl_palloc palloc, memory_allocator_nginx_impl_pcopy pcopy, memory_allocator_nginx_impl_pfree pfree);

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_ALLOCATOR_NGINX_H */
