/*
 *	Portability Library by Parra Studios
 *	A generic cross-platform portability utility.
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

#ifndef PORTABILITY_PATH_H
#define PORTABILITY_PATH_H 1

/* -- Headers -- */

#include <portability/portability_api.h>

/* -- Definitions -- */

/* Path limits */
#if defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)

	#ifndef NOMINMAX
		#define NOMINMAX
	#endif

	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif

	#include <windows.h>
	#define PORTABILITY_PATH_SIZE MAX_PATH
#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__NetBSD__) || defined(__DragonFly__)

	#include <limits.h>
	#include <unistd.h>

	#define PORTABILITY_PATH_SIZE PATH_MAX
#elif (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
	#include <stdlib.h>
	#include <limits.h>
	#include <sys/syslimits.h>

	#define PORTABILITY_PATH_SIZE PATH_MAX
#elif defined(__FreeBSD__)
	#include <limits.h>
	#include <sys/types.h>
	#include <sys/sysctl.h>

	#define PORTABILITY_PATH_SIZE PATH_MAX
#elif defined(sun) || defined(__sun)
	#include <stdlib.h>
	#include <limits.h>
	#include <string.h>

	#define PORTABILITY_PATH_SIZE PATH_MAX
#else
	#error "Unimplemented platform, please add support to it"
#endif

/* Path separator */
#if defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)

	#define PORTABILITY_PATH_SEPARATOR(chr) (chr == '\\' || chr == '/')
	#define PORTABILITY_PATH_SEPARATOR_C	'/'

#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__) || \
	defined(__HAIKU__) || defined(__BEOS__)
	#define PORTABILITY_PATH_SEPARATOR(chr) (chr == '/')
	#define PORTABILITY_PATH_SEPARATOR_C	'/'

#else
	#error "Unknown path separator"
#endif

/* Path delimiter */
#if defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)

	#define PORTABILITY_PATH_DELIMITER ';'

#else
	#define PORTABILITY_PATH_DELIMITER ':'
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
 * @brief
 *   Get the file name portion out of a path and strip away the file extension if any.
 *
 *   If `path` is NULL this will return an empty string.
 *
 *   If `name` is NULL or `name_size is 0 this will return the size it requires in order to write `name`.
 *
 *   If `path` or `name` are not NULL, then `path_size` or `name_size`, respectively, must be set to <= the length (including 0-terminator) of the memory regions pointed to by `path` and `name`.
 * @param[in] path
 *   The full path to extract the name from.
 * @param[in] path_size
 *   The length (including 0-terminator) of `path` in chars.
 * @param[out] name
 *   The memory location to write the extracted name to. If `NULL` the size required will be returned instead of the size written.
 * @param[in] name_size
 *   The size of the memory location pointed to by `name`.
 * @return
 *   The size of the name.
 */
PORTABILITY_API size_t portability_path_get_name(const char *const path, const size_t path_size, char *const name, const size_t name_size);

/**
 * @brief
 *   Get the file name portion out of a path and strip away any amount of file extensions.
 *
 *   When called with `"/foo/bar.baz.qux"`:
 *
 *     - `portability_path_get_name` will produce the string `"bar.baz"`
 *
 *     - `portability_path_get_name_canonical` will produce the string `"bar"`
 *
 *   If `path` is NULL this will return an empty string.
 *
 *   If `name` is NULL or `name_size is 0 this will return the size it requires in order to write `name`.
 *
 *   If `path` or `name` are not NULL, then `path_size` or `name_size`, respectively, must be set to <= the length (including 0-terminator) of the memory regions pointed to by `path` and `name`.
 * @param[in] path
 *   The full path to extract the name from.
 * @param[in] path_size
 *   The length (including 0-terminator) of `path` in chars.
 * @param[out] name
 *   The memory location to write the extracted name to. If `NULL` the size required will be returned instead of the size written.
 * @param[in] name_size
 *   The size of the memory location pointed to by `name`.
 * @return
 *   The size of the name.
 */
PORTABILITY_API size_t portability_path_get_name_canonical(const char *const path, const size_t path_size, char *const name, const size_t name_size);

PORTABILITY_API size_t portability_path_get_fullname(const char *path, size_t path_size, char *name, size_t name_size);

PORTABILITY_API size_t portability_path_get_extension(const char *path, size_t path_size, char *extension, size_t extension_size);

PORTABILITY_API size_t portability_path_get_module_name(const char *path, size_t path_size, const char *extension, size_t extension_size, char *name, size_t name_size);

PORTABILITY_API size_t portability_path_get_directory(const char *path, size_t path_size, char *absolute, size_t absolute_size);

PORTABILITY_API size_t portability_path_get_directory_inplace(char *path, size_t size);

PORTABILITY_API size_t portability_path_get_relative(const char *base, size_t base_size, const char *path, size_t path_size, char *relative, size_t relative_size);

PORTABILITY_API int portability_path_is_subpath(const char *parent, size_t parent_size, const char *child, size_t child_size);

PORTABILITY_API int portability_path_is_absolute(const char *path, size_t size);

PORTABILITY_API size_t portability_path_join(const char *left_path, size_t left_path_size, const char *right_path, size_t right_path_size, char *join_path, size_t join_size);

PORTABILITY_API size_t portability_path_canonical(const char *path, size_t path_size, char *canonical, size_t canonical_size);

PORTABILITY_API int portability_path_separator_normalize_inplace(char *path, size_t size);

PORTABILITY_API int portability_path_compare(const char *left_path, const char *right_path);

PORTABILITY_API int portability_path_is_pattern(const char *path, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* PORTABILITY_PATH_H */
