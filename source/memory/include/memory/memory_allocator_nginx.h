/*
 *	Memory Library by Parra Studios
 *	A generic cross-platform memory utility.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

MEMORY_API memory_allocator memory_allocator_nginx(void *pool, memory_allocator_nginx_impl_palloc palloc, memory_allocator_nginx_impl_pcopy pcopy, memory_allocator_nginx_impl_pfree pfree);

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_ALLOCATOR_NGINX_H */
