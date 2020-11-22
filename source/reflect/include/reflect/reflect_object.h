/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef REFLECT_OBJECT_H
#define REFLECT_OBJECT_H 1

#include <reflect/reflect_value.h>

#ifdef __cplusplus
extern "C" {
#endif

struct object_type;

typedef void * object_impl;

typedef struct object_type * object;

#include <reflect/reflect_class.h>

typedef void * object_args[];

typedef value (*object_resolve_callback)(value, void *);

typedef value (*object_reject_callback)(value, void *);

typedef int (*object_impl_interface_create)(object, object_impl);

typedef value (*object_impl_interface_get)(object, object_impl, const char *);

typedef int (*object_impl_interface_set)(object, object_impl, const char *, value);

typedef value (*object_impl_interface_method_invoke)(object, object_impl, const char *, object_args, size_t);

typedef value (*object_impl_interface_method_await)(object, object_impl, const char *, object_args, size_t, object_resolve_callback, object_reject_callback, void *);

typedef int (*object_impl_interface_destructor)(object, object_impl);

typedef void (*object_impl_interface_destroy)(object, object_impl);

typedef struct object_interface_type
{
    object_impl_interface_create create;
	object_impl_interface_get get;
	object_impl_interface_set set;
	object_impl_interface_method_invoke method_invoke;
	object_impl_interface_method_await method_await;
    object_impl_interface_destructor destructor;
    object_impl_interface_destroy destroy;

} * object_interface;

typedef object_interface (*object_impl_interface_singleton)(void);

REFLECT_API object object_create(const char * name, object_impl impl, object_impl_interface_singleton singleton, klass cls);

REFLECT_API int object_increment_reference(object obj);

REFLECT_API int object_decrement_reference(object obj);

REFLECT_API object_impl object_impl_get(object obj);

REFLECT_API int object_set(object obj, const char * key, value v);

REFLECT_API value object_get(object obj, const char * key);

REFLECT_API value object_call(object obj, const char * name, object_args args, size_t size);

REFLECT_API value object_await(object obj, const char * name, object_args args, size_t size, object_resolve_callback resolve_callback, object_reject_callback reject_callback, void * context);

REFLECT_API const char * object_name(object obj);

REFLECT_API value object_metadata(object obj);

REFLECT_API int object_delete(object obj);

REFLECT_API void object_destroy(object obj);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_OBJECT_H */
