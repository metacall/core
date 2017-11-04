/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

 /* -- Headers -- */

#include <metacall/metacall_allocator.h>

#include <memory/memory.h>

/* -- Methods -- */

void * metacall_allocator(enum metacall_allocator_id allocator_id, void * ctx)
{
	switch (allocator_id)
	{
		case METACALL_ALLOCATOR_STD :
		{
			metacall_allocator_std std_ctx = (metacall_allocator_std)ctx;

			return memory_allocator_std(std_ctx->std_malloc, std_ctx->std_realloc, std_ctx->std_free);
		}

		case METACALL_ALLOCATOR_NGINX :
		{
			metacall_allocator_nginx nginx_ctx = (metacall_allocator_nginx)ctx;

			return memory_allocator_nginx(nginx_ctx->pool, nginx_ctx->nginx_palloc, nginx_ctx->nginx_pfree);
		}
	}

	return NULL;
}
