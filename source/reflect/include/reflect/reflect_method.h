/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef REFLECT_METHOD_H
#define REFLECT_METHOD_H 1

#include <reflect/reflect_api.h>

#include <reflect/reflect_class_decl.h>
#include <reflect/reflect_class_visibility.h>

#include <reflect/reflect_async.h>
#include <reflect/reflect_signature.h>

#include <reflect/reflect_method_decl.h>

#ifdef __cplusplus
extern "C" {
#endif

REFLECT_API method method_create(klass cls, const char *name, size_t args_count, method_impl impl, enum class_visibility_id visibility, enum async_id async, method_impl_interface_singleton singleton);

REFLECT_API klass method_class(method m);

REFLECT_API char *method_name(method m);

REFLECT_API signature method_signature(method m);

REFLECT_API method_impl method_data(method m);

REFLECT_API enum class_visibility_id method_visibility(method m);

REFLECT_API enum async_id method_async(method m);

REFLECT_API value method_metadata(method m);

REFLECT_API void method_destroy(method m);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_METHOD_H */
