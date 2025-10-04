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

/* -- Headers -- */

#include <environment/environment_variable_path.h>

#include <stdlib.h>
#include <string.h>

/* -- Definitions -- */

#if defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)
	#define ENVIRONMENT_VARIABLE_PATH_SEPARATOR(chr) (chr == '/' || chr == '\\')
#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__) || \
	defined(__HAIKU__) || defined(__BEOS__)
	#define ENVIRONMENT_VARIABLE_PATH_SEPARATOR(chr) (chr == '/')
#else
	#error "Unknown environment variable path separator"
#endif

/* -- Methods -- */

char *environment_variable_path_create(const char *const name, const char *const default_path, const size_t default_path_size, size_t *const env_size)
{
	const char *env_variable = getenv(name);
	char *path;
	size_t size, alloc_size;

	if (env_variable)
	{
		size = strlen(env_variable) + 1;
	}
	else if (default_path)
	{
		env_variable = default_path;
		size = default_path_size;
	}
	else
	{
		env_variable = ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR;
		size = sizeof(ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR);
	}

	alloc_size = size;

	if (size > 1)
	{
		alloc_size += !ENVIRONMENT_VARIABLE_PATH_SEPARATOR(env_variable[size - 2]);
	}

	path = malloc(sizeof(char) * alloc_size);

	if (path == NULL)
	{
		return NULL;
	}

	memcpy(path, env_variable, sizeof(char) * size);

	if (size > 1)
	{
		path[alloc_size - 2] = ENVIRONMENT_VARIABLE_PATH_SEPARATOR_C;
	}

	path[alloc_size - 1] = '\0';

	if (env_size)
	{
		*env_size = alloc_size;
	}

	return path;
}

void environment_variable_path_destroy(char *variable_path)
{
	if (variable_path)
	{
		free(variable_path);
	}
}
