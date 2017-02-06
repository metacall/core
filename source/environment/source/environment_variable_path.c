/*
 *	Environment Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for supporting formatted input / output.
 *
 */

/* -- Headers -- */

#include <environment/environment_variable_path.h>

#include <stdlib.h>
#include <string.h>

/* -- Definitions -- */

#if defined(_WIN32)
#	define ENVIRONMENT_VARIABLE_PATH_SEPARATOR '\\'
#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
#	define ENVIRONMENT_VARIABLE_PATH_SEPARATOR '/'
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

	if (path_ptr[last] == ENVIRONMENT_VARIABLE_PATH_SEPARATOR)
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

	path[last] = ENVIRONMENT_VARIABLE_PATH_SEPARATOR;
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
