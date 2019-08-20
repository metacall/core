/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#ifndef REFLECT_FUTURE_H
#define REFLECT_FUTURE_H 1

#include <reflect/reflect_value.h>

#ifdef __cplusplus
extern "C" {
#endif

struct future_type;

typedef void * future_impl;

typedef struct future_type * future;

typedef value (*future_resolve_callback)(value, void *);

typedef value (*future_reject_callback)(value, void *);

typedef value future_return;

typedef int (*future_impl_interface_create)(future, future_impl);

typedef future_return (*future_impl_interface_await)(future, future_impl, future_resolve_callback, future_reject_callback, void *);

typedef void (*future_impl_interface_destroy)(future, future_impl);

typedef struct future_interface_type
{
	future_impl_interface_create create;
	future_impl_interface_await await;
	future_impl_interface_destroy destroy;

} * future_interface;

typedef future_interface (*future_impl_interface_singleton)(void);

REFLECT_API future future_create(future_impl impl, future_impl_interface_singleton singleton);

REFLECT_API future_return future_await(future f, future_resolve_callback resolve_callback, future_reject_callback reject_callback, void * data);

REFLECT_API void future_destroy(future f);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_FUTURE_H */
