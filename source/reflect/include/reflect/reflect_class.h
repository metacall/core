/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <adt/adt_vector.h>

#include <reflect/reflect_accessor.h>
#include <reflect/reflect_attribute.h>
#include <reflect/reflect_class_decl.h>
#include <reflect/reflect_class_visibility.h>
#include <reflect/reflect_constructor.h>
#include <reflect/reflect_method.h>
#include <reflect/reflect_object.h>
#include <reflect/reflect_signature.h>
#include <reflect/reflect_value.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *class_args[];

typedef value (*class_resolve_callback)(value, void *);

typedef value (*class_reject_callback)(value, void *);

typedef int (*class_impl_interface_create)(klass, class_impl);

typedef object (*class_impl_interface_constructor)(klass, class_impl, const char *, constructor, class_args, size_t);

typedef value (*class_impl_interface_static_get)(klass, class_impl, struct accessor_type *);

typedef int (*class_impl_interface_static_set)(klass, class_impl, struct accessor_type *, value);

typedef value (*class_impl_interface_static_invoke)(klass, class_impl, method, class_args, size_t);

typedef value (*class_impl_interface_static_await)(klass, class_impl, method, class_args, size_t, class_resolve_callback, class_reject_callback, void *);

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

REFLECT_API klass class_create(const char *name, enum accessor_type_id accessor, class_impl impl, class_impl_interface_singleton singleton);

REFLECT_API int class_increment_reference(klass cls);

REFLECT_API int class_decrement_reference(klass cls);

REFLECT_API class_impl class_impl_get(klass cls);

REFLECT_API object class_new(klass cls, const char *name, constructor ctor, class_args args, size_t argc);

REFLECT_API value class_static_get(klass cls, const char *key);

REFLECT_API int class_static_set(klass cls, const char *key, value v);

REFLECT_API vector class_constructors(klass cls);

REFLECT_API constructor class_default_constructor(klass cls);

REFLECT_API constructor class_constructor(klass cls, type_id args[], size_t size);

REFLECT_API vector class_static_methods(klass cls, const char *key);

REFLECT_API vector class_methods(klass cls, const char *key);

REFLECT_API method class_static_method(klass cls, const char *key, type_id ret, type_id args[], size_t size);

REFLECT_API method class_method(klass cls, const char *key, type_id ret, type_id args[], size_t size);

REFLECT_API vector class_method_names(klass cls);

REFLECT_API vector class_static_method_names(klass cls);

REFLECT_API attribute class_static_attribute(klass cls, const char *key);

REFLECT_API attribute class_attribute(klass cls, const char *key);

REFLECT_API vector class_attribute_names(klass cls);

REFLECT_API vector class_static_attribute_names(klass cls);

REFLECT_API int class_register_constructor(klass cls, constructor ctor);

REFLECT_API int class_register_static_method(klass cls, method m);

REFLECT_API int class_register_method(klass cls, method m);

REFLECT_API int class_register_static_attribute(klass cls, attribute attr);

REFLECT_API int class_register_attribute(klass cls, attribute attr);

REFLECT_API value class_static_call(klass cls, method m, class_args args, size_t size);

REFLECT_API value class_static_await(klass cls, method m, class_args args, size_t size, class_resolve_callback resolve_callback, class_reject_callback reject_callback, void *context);

REFLECT_API const char *class_name(klass cls);

REFLECT_API value class_metadata(klass cls);

REFLECT_API void class_stats_debug(void);

REFLECT_API void class_destroy(klass cls);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_CLASS_H */
