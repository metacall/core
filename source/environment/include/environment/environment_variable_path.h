/*
 *	Environment Library by Parra Studios
 *	A cross-platform library for supporting platform specific environment features.
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

#ifndef ENVIRONMENT_VARIABLE_PATH_H
#define ENVIRONMENT_VARIABLE_PATH_H 1

/* -- Headers -- */

#include <environment/environment_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stddef.h>

/* -- Definitions -- */

#if defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)
	#define ENVIRONMENT_VARIABLE_PATH_SEPARATOR_C	'\\'
	#define ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR "\\"
#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__) || \
	defined(__HAIKU__) || defined(__BEOS__)
	#define ENVIRONMENT_VARIABLE_PATH_SEPARATOR_C	'/'
	#define ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR "/"
#else
	#error "Unknown environment variable path separator"
#endif

/* -- Methods -- */

/**
 * @brief
 *   If the value of @name exists as an environment variable, return a live string of its value, otherwise return a live value of @default_path or "/".
 *   @name should not be NULL.
 *   If @default_path is not NULL, @default_path_size must be set to <= the length (including null-terminator) of the @default_path string.
 *   If @env_size is not NULL, the length (including null-terminator) of the returned string will be set to it.
 *
 * @param[in] name
 *   The environment variable name to look up.
 *
 * @param[in] default_path
 *   If the environment variable value is not found, the value to return instead.
 *
 * @param[in] default_path_size
 *   The length (including null-terminator) of @default_path in chars.
 *
 * @param[out] env_size
 *   Pointer to a size_t to write the length of the returned string to (optional).
 *
 * @return
 *   The allocated string containing the environment variable value or the default or "/".
 */
ENVIRONMENT_API char *environment_variable_path_create(const char *const name, const char *const default_path, const size_t default_path_size, size_t *const env_size);

ENVIRONMENT_API void environment_variable_path_destroy(char *variable_path);

#ifdef __cplusplus
}
#endif

#endif /* ENVIRONMENT_VARIABLE_PATH_H */
