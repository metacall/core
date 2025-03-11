/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef REFLECT_EXCEPTION_H
#define REFLECT_EXCEPTION_H 1

#include <reflect/reflect_api.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct exception_type;

typedef struct exception_type *exception;

REFLECT_API exception exception_create(char *message, char *label, int64_t code, char *stacktrace);

REFLECT_API exception exception_create_const(const char *message, const char *label, int64_t code, const char *stacktrace);

REFLECT_API int exception_increment_reference(exception ex);

REFLECT_API int exception_decrement_reference(exception ex);

REFLECT_API const char *exception_message(exception ex);

REFLECT_API const char *exception_label(exception ex);

REFLECT_API int64_t exception_error_code(exception ex);

REFLECT_API const char *exception_stacktrace(exception ex);

REFLECT_API void exception_stats_debug(void);

REFLECT_API void exception_destroy(exception ex);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_EXCEPTION_H */
