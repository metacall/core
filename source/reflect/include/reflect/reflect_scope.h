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

#ifndef REFLECT_SCOPE_H
#define REFLECT_SCOPE_H 1

#include <reflect/reflect_api.h>

#include <reflect/reflect_function.h>
#include <reflect/reflect_type.h>

#ifdef __cplusplus
extern "C" {
#endif

struct scope_type;

typedef size_t scope_stack_ptr;

typedef struct scope_type *scope;

REFLECT_API scope scope_create(const char *name);

REFLECT_API size_t scope_size(scope sp);

REFLECT_API int scope_define(scope sp, const char *key, value obj);

REFLECT_API value scope_metadata(scope sp);

REFLECT_API value scope_export(scope sp);

REFLECT_API value scope_get(scope sp, const char *key);

REFLECT_API value scope_undef(scope sp, const char *key);

REFLECT_API int scope_append(scope dest, scope src);

REFLECT_API int scope_contains(scope dest, scope src, char **duplicated);

REFLECT_API int scope_remove(scope dest, scope src);

REFLECT_API size_t *scope_stack_return(scope sp);

REFLECT_API scope_stack_ptr scope_stack_push(scope sp, size_t bytes);

REFLECT_API void *scope_stack_get(scope sp, scope_stack_ptr stack_ptr);

REFLECT_API int scope_stack_pop(scope sp);

REFLECT_API void scope_destroy(scope sp);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_SCOPE_H */
