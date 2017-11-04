/*
 *	Memory Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic cross-platform memory utility.
 *
 */

#ifndef MEMORY_ALLOCATOR_NGINX_IMPL_H
#define MEMORY_ALLOCATOR_NGINX_IMPL_H 1

/* -- Headers -- */

#include <memory/memory_api.h>

#include <memory/memory_allocator_iface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct memory_allocator_nginx_ctx_type;

/* -- Type Definitions -- */

typedef void * (*memory_allocator_nginx_impl_palloc)(void *, size_t);

typedef int (*memory_allocator_nginx_impl_pfree)(void *, void *);

typedef struct memory_allocator_nginx_ctx_type * memory_allocator_nginx_ctx;

/* -- Member Data -- */

struct memory_allocator_nginx_ctx_type
{
	void * pool;
	memory_allocator_nginx_impl_palloc palloc;
	memory_allocator_nginx_impl_pfree pfree;
};

/* -- Methods -- */

MEMORY_API memory_allocator_iface memory_allocator_nginx_iface(void);

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_ALLOCATOR_NGINX_IMPL_H */
