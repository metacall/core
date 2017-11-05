/*
 *	Memory Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic cross-platform memory utility.
 *
 */

/* -- Headers -- */


#include <memory/memory_allocator_std_impl.h>

#include <stdlib.h>

/* -- Forward Declarations -- */

struct memory_allocator_std_impl_type;

/* -- Type Definitions -- */

typedef struct memory_allocator_std_impl_type * memory_allocator_std_impl;

/* -- Member Data -- */

struct memory_allocator_std_impl_type
{
	memory_allocator_std_impl_malloc malloc;
	memory_allocator_std_impl_realloc realloc;
	memory_allocator_std_impl_free free;
};

/* -- Private Methods -- */

static memory_allocator_impl memory_allocator_std_create(void * ctx);

static void * memory_allocator_std_allocate(memory_allocator_impl impl, size_t size);

static void * memory_allocator_std_reallocate(memory_allocator_impl impl, void * data, size_t size, size_t new_size);

static void memory_allocator_std_deallocate(memory_allocator_impl impl, void * data);

static void memory_allocator_std_destroy(memory_allocator_impl impl);

/* -- Methods -- */

memory_allocator_iface memory_allocator_std_iface()
{
	static struct memory_allocator_iface_type allocator_std_iface =
	{
		&memory_allocator_std_create,
		&memory_allocator_std_allocate,
		&memory_allocator_std_reallocate,
		&memory_allocator_std_deallocate,
		&memory_allocator_std_destroy
	};

	return &allocator_std_iface;
}

memory_allocator_impl memory_allocator_std_create(void * ctx)
{
	memory_allocator_std_ctx std_ctx = (memory_allocator_std_ctx)ctx;

	memory_allocator_std_impl std_impl = std_ctx->malloc(sizeof(struct memory_allocator_std_impl_type));

	if (std_impl == NULL)
	{
		return NULL;
	}

	std_impl->malloc = std_ctx->malloc;
	std_impl->realloc = std_ctx->realloc;
	std_impl->free = std_ctx->free;

	return (memory_allocator_impl)std_impl;
}

void * memory_allocator_std_allocate(memory_allocator_impl impl, size_t size)
{
	memory_allocator_std_impl std_impl = (memory_allocator_std_impl)impl;

	return std_impl->malloc(size);
}

void * memory_allocator_std_reallocate(memory_allocator_impl impl, void * data, size_t size, size_t new_size)
{
	memory_allocator_std_impl std_impl = (memory_allocator_std_impl)impl;

	(void)size;

	return std_impl->realloc(data, new_size);
}

void memory_allocator_std_deallocate(memory_allocator_impl impl, void * data)
{
	memory_allocator_std_impl std_impl = (memory_allocator_std_impl)impl;

	std_impl->free(data);
}

void memory_allocator_std_destroy(memory_allocator_impl impl)
{
	memory_allocator_std_impl std_impl = (memory_allocator_std_impl)impl;

	memory_allocator_std_impl_free free = std_impl->free;

	free(std_impl);
}
