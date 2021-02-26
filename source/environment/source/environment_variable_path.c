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

/* -- Headers -- */

#include <environment/environment_variable_path.h>

#include <stdlib.h>
#include <string.h>

/* -- Definitions -- */

#if defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)
#	define ENVIRONMENT_VARIABLE_PATH_SEPARATOR(chr) (chr == '/' || chr == '\\')
#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__) || \
	defined(__HAIKU__) || defined(__BEOS__)
#	define ENVIRONMENT_VARIABLE_PATH_SEPARATOR(chr) (chr == '/')
#else
#	error "Unknown environment variable path separator"
#endif

/* -- Methods -- */

char * environment_variable_path_create(const char * name, const char * default_path)
{
	const char * path_ptr = getenv(name);

	char * path;

	size_t length, size, last, end;

	if (path_ptr == NULL)
	{
		if (default_path == NULL)
		{
			static const char empty_path[] = "";

			default_path = empty_path;
		}

		path_ptr = default_path;
	}

	length = strlen(path_ptr);

	last = length - 1;

	if (ENVIRONMENT_VARIABLE_PATH_SEPARATOR(path_ptr[last]))
	{
		end = length;
		size = length + 1;
	}
	else
	{
		last = length;
		end = length + 1;
		size = length + 2;
	}

	path = malloc(sizeof(char) * size);

	if (path == NULL)
	{
		return NULL;
	}

	strncpy(path, path_ptr, length);

	path[last] = ENVIRONMENT_VARIABLE_PATH_SEPARATOR_C;
	path[end] = '\0';

	return path;
}

void environment_variable_path_destroy(char * variable_path)
{
	if (variable_path)
	{
		free(variable_path);
	}
}
