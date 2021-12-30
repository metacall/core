/*
 *	Environment Library by Parra Studios
 *	A cross-platform library for supporting platform specific environment features.
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

#ifndef ENVIRONMENT_VARIABLE_H
#define ENVIRONMENT_VARIABLE_H 1

/* -- Headers -- */

#include <environment/environment_api.h>

#include <preprocessor/preprocessor_stringify.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Macros -- */

#define environment_variable_set_expand(definition) \
	environment_variable_set(PREPROCESSOR_STRINGIFY(definition), definition)

/* -- Methods -- */

ENVIRONMENT_API char *environment_variable_create(const char *name, const char *default_value);

ENVIRONMENT_API const char *environment_variable_get(const char *name, const char *default_value);

ENVIRONMENT_API int environment_variable_set(const char *name, const char *value_string);

ENVIRONMENT_API void environment_variable_destroy(char *variable);

#ifdef __cplusplus
}
#endif

#endif /* ENVIRONMENT_VARIABLE_H */
