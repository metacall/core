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

#ifndef REFLECT_CONSTRUCTOR_H
#define REFLECT_CONSTRUCTOR_H 1

#include <reflect/reflect_api.h>

#include <reflect/reflect_class_visibility.h>
#include <reflect/reflect_type.h>
#include <reflect/reflect_value.h>

#include <reflect/reflect_constructor_decl.h>

#ifdef __cplusplus
extern "C" {
#endif

REFLECT_API constructor constructor_create(size_t count, enum class_visibility_id visibility);

REFLECT_API size_t constructor_count(constructor ctor);

REFLECT_API size_t constructor_get_index(constructor ctor, const char *name);

REFLECT_API const char *constructor_get_name(constructor ctor, size_t index);

REFLECT_API type constructor_get_type(constructor ctor, size_t index);

REFLECT_API void constructor_set(constructor ctor, size_t index, const char *name, type t);

REFLECT_API int constructor_compare(constructor ctor, type_id args[], size_t size);

REFLECT_API enum class_visibility_id constructor_visibility(constructor ctor);

REFLECT_API value constructor_metadata(constructor ctor);

REFLECT_API void constructor_destroy(constructor ctor);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_CONSTRUCTOR_H */
