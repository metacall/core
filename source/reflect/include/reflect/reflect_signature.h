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

#ifndef REFLECT_SIGNATURE_H
#define REFLECT_SIGNATURE_H 1

#include <reflect/reflect_api.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_value.h>

#ifdef __cplusplus
extern "C" {
#endif

struct signature_type;

typedef struct signature_type *signature;

REFLECT_API signature signature_create(size_t count);

REFLECT_API signature signature_resize(signature s, size_t count);

REFLECT_API size_t signature_count(signature s);

REFLECT_API size_t signature_get_index(signature s, const char *name);

REFLECT_API const char *signature_get_name(signature s, size_t index);

REFLECT_API type signature_get_type(signature s, size_t index);

REFLECT_API type signature_get_return(signature s);

REFLECT_API void signature_set(signature s, size_t index, const char *name, type t);

REFLECT_API void signature_set_return(signature s, type t);

REFLECT_API int signature_compare(signature s, type_id ret, type_id args[], size_t size);

REFLECT_API value signature_metadata(signature s);

REFLECT_API void signature_destroy(signature s);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_SIGNATURE_H */
