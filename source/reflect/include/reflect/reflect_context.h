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

#ifndef REFLECT_CONTEXT_H
#define REFLECT_CONTEXT_H 1

#include <reflect/reflect_api.h>

#include <reflect/reflect_scope.h>

#ifdef __cplusplus
extern "C" {
#endif

struct context_type;

typedef struct context_type *context;

REFLECT_API context context_create(const char *name);

REFLECT_API const char *context_name(context ctx);

REFLECT_API scope context_scope(context ctx);

REFLECT_API int context_append(context dest, context src);

REFLECT_API int context_contains(context dest, context src, char **duplicated);

REFLECT_API int context_remove(context dest, context src);

REFLECT_API void context_destroy(context ctx);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_CONTEXT_H */
