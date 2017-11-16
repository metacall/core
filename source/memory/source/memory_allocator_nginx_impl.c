/*
 *	Memory Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic cross-platform memory utility.
 *
 */

/* -- Headers -- */

#include <memory/memory_allocator_nginx_impl.h>

#include <log/log.h>

/* -- Forward Declarations -- */

struct memory_allocator_nginx_impl_type;

/* -- Type Definitions -- */

typedef struct memory_allocator_nginx_impl_type * memory_allocator_nginx_impl;

/* -- Member Data -- */

struct memory_allocator_nginx_impl_type
{
	void * pool;
	memory_allocator_nginx_impl_palloc palloc;
	memory_allocator_nginx_impl_pcopy pcopy;
	memory_allocator_nginx_impl_pfree pfree;
};

/* -- Private Methods -- */

static memory_allocator_impl memory_allocator_nginx_create(void * ctx);

static void * memory_allocator_nginx_allocate(memory_allocator_impl impl, size_t size);

static void * memory_allocator_nginx_reallocate(memory_allocator_impl impl, void * data, size_t size, size_t new_size);

static void memory_allocator_nginx_deallocate(memory_allocator_impl impl, void * data);

static void memory_allocator_nginx_destroy(memory_allocator_impl impl);

/* -- Methods -- */

memory_allocator_iface memory_allocator_nginx_iface()
{
	static struct memory_allocator_iface_type allocator_nginx_iface =
	{
		&memory_allocator_nginx_create,
		&memory_allocator_nginx_allocate,
		&memory_allocator_nginx_reallocate,
		&memory_allocator_nginx_deallocate,
		&memory_allocator_nginx_destroy
	};

	return &allocator_nginx_iface;
}

memory_allocator_impl memory_allocator_nginx_create(void * ctx)
{
	memory_allocator_nginx_ctx nginx_ctx = (memory_allocator_nginx_ctx)ctx;

	memory_allocator_nginx_impl nginx_impl = nginx_ctx->palloc(nginx_ctx->pool, sizeof(struct memory_allocator_nginx_impl_type));

	if (nginx_impl == NULL)
	{
		return NULL;
	}

	nginx_impl->pool = nginx_ctx->pool;
	nginx_impl->palloc = nginx_ctx->palloc;
	nginx_impl->pcopy = nginx_ctx->pcopy;
	nginx_impl->pfree = nginx_ctx->pfree;

	return (memory_allocator_impl)nginx_impl;
}

void * memory_allocator_nginx_allocate(memory_allocator_impl impl, size_t size)
{
	memory_allocator_nginx_impl nginx_impl = (memory_allocator_nginx_impl)impl;

	return nginx_impl->palloc(nginx_impl->pool, size);
}

void * memory_allocator_nginx_reallocate(memory_allocator_impl impl, void * data, size_t size, size_t new_size)
{
	memory_allocator_nginx_impl nginx_impl = (memory_allocator_nginx_impl)impl;

	void * new_data = nginx_impl->palloc(nginx_impl->pool, new_size);

	if (new_data == NULL)
	{
		return NULL;
	}

	if (size < new_size)
	{
		nginx_impl->pcopy(new_data, data, size);
	}
	else
	{
		nginx_impl->pcopy(new_data, data, new_size);
	}

	(void)nginx_impl->pfree(nginx_impl->pool, data);

	return new_data;
}

void memory_allocator_nginx_deallocate(memory_allocator_impl impl, void * data)
{
	memory_allocator_nginx_impl nginx_impl = (memory_allocator_nginx_impl)impl;

	(void)nginx_impl->pfree(nginx_impl->pool, data);
}

void memory_allocator_nginx_destroy(memory_allocator_impl impl)
{
	memory_allocator_nginx_impl nginx_impl = (memory_allocator_nginx_impl)impl;

	void * pool = nginx_impl->pool;

	memory_allocator_nginx_impl_pfree pfree = nginx_impl->pfree;

	(void)pfree(pool, nginx_impl);
}
