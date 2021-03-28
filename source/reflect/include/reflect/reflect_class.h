/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#ifndef REFLECT_CLASS_H
#define REFLECT_CLASS_H 1

#include <reflect/reflect_value.h>

#ifdef __cplusplus
extern "C" {
#endif

struct class_type;

typedef void *class_impl;

typedef struct class_type *klass;

#include <reflect/reflect_object.h>

typedef void *class_args[];

typedef value (*class_resolve_callback)(value, void *);

typedef value (*class_reject_callback)(value, void *);

typedef int (*class_impl_interface_create)(klass, class_impl);

typedef object (*class_impl_interface_constructor)(klass, class_impl, const char *name, class_args, size_t);

typedef value (*class_impl_interface_static_get)(klass, class_impl, const char *);

typedef int (*class_impl_interface_static_set)(klass, class_impl, const char *, value);

typedef value (*class_impl_interface_static_invoke)(klass, class_impl, const char *, class_args, size_t);

typedef value (*class_impl_interface_static_await)(klass, class_impl, const char *, class_args, size_t, class_resolve_callback, class_reject_callback, void *);

typedef void (*class_impl_interface_destroy)(klass, class_impl);

typedef struct class_interface_type
{
	class_impl_interface_create create;
	class_impl_interface_constructor constructor;
	class_impl_interface_static_get static_get;
	class_impl_interface_static_set static_set;
	class_impl_interface_static_invoke static_invoke;
	class_impl_interface_static_await static_await;
	class_impl_interface_destroy destroy;

} * class_interface;

typedef class_interface (*class_impl_interface_singleton)(void);

REFLECT_API klass class_create(const char *name, class_impl impl, class_impl_interface_singleton singleton);

REFLECT_API int class_increment_reference(klass cls);

REFLECT_API int class_decrement_reference(klass cls);

REFLECT_API class_impl class_impl_get(klass cls);

REFLECT_API object class_new(klass cls, const char *name, class_args args, size_t argc);

REFLECT_API value class_static_get(klass cls, const char *key);

REFLECT_API int class_static_set(klass cls, const char *key, value v);

REFLECT_API value class_static_call(klass cls, const char *name, class_args args, size_t size);

REFLECT_API value class_static_await(klass cls, const char *name, class_args args, size_t size, class_resolve_callback resolve_callback, class_reject_callback reject_callback, void *context);

REFLECT_API const char *class_name(klass cls);

REFLECT_API value class_metadata(klass cls);

REFLECT_API void class_destroy(klass cls);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_CLASS_H */
