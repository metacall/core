/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

/* -- Headers -- */

#include <metacall/metacall_allocator.h>

#include <memory/memory.h>

/* -- Methods -- */

void *metacall_allocator_create(enum metacall_allocator_id allocator_id, void *ctx)
{
	switch (allocator_id)
	{
		case METACALL_ALLOCATOR_STD: {
			metacall_allocator_std std_ctx = (metacall_allocator_std)ctx;

			return memory_allocator_std(std_ctx->malloc, std_ctx->realloc, std_ctx->free);
		}

		case METACALL_ALLOCATOR_NGINX: {
			metacall_allocator_nginx nginx_ctx = (metacall_allocator_nginx)ctx;

			return memory_allocator_nginx((void *)nginx_ctx->pool,
				(memory_allocator_nginx_impl_palloc)nginx_ctx->palloc,
				(memory_allocator_nginx_impl_pcopy)nginx_ctx->pcopy,
				(memory_allocator_nginx_impl_pfree)nginx_ctx->pfree);
		}
	}

	return NULL;
}

void *metacall_allocator_alloc(void *allocator, size_t size)
{
	return memory_allocator_allocate((memory_allocator)allocator, size);
}

void *metacall_allocator_realloc(void *allocator, void *data, size_t size, size_t new_size)
{
	return memory_allocator_reallocate((memory_allocator)allocator, data, size, new_size);
}

void metacall_allocator_free(void *allocator, void *data)
{
	memory_allocator_deallocate((memory_allocator)allocator, data);
}

void metacall_allocator_destroy(void *allocator)
{
	memory_allocator_destroy((memory_allocator)allocator);
}
