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

#ifndef REFLECT_ATTRIBUTE_H
#define REFLECT_ATTRIBUTE_H 1

#include <reflect/reflect_api.h>

#include <reflect/reflect_class_decl.h>
#include <reflect/reflect_class_visibility.h>

#include <reflect/reflect_type.h>

#include <reflect/reflect_attribute_decl.h>

#ifdef __cplusplus
extern "C" {
#endif

REFLECT_API attribute attribute_create(klass cls, const char *name, type t, attribute_impl impl, enum class_visibility_id visibility, attribute_impl_interface_singleton singleton);

REFLECT_API klass attribute_class(attribute attr);

REFLECT_API char *attribute_name(attribute attr);

REFLECT_API type attribute_type(attribute attr);

REFLECT_API attribute_impl attribute_data(attribute attr);

REFLECT_API enum class_visibility_id attribute_visibility(attribute attr);

REFLECT_API value attribute_metadata(attribute attr);

REFLECT_API void attribute_destroy(attribute attr);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_ATTRIBUTE_H */
