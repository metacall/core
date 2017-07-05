/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#include <loader/loader_path.h>

#if defined(_WIN32)
#	define LOADER_PATH_SEPARATOR '\\'
#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
#	define LOADER_PATH_SEPARATOR '/'
#else
#	error "Unknown loader path separator"
#endif

size_t loader_path_get_name(const loader_naming_path path, loader_naming_name name)
{
	size_t i, count, last;

	for (i = 0, count = 0, last = 0; path[i] != '\0' &&
		i < LOADER_NAMING_PATH_SIZE /*&& count < LOADER_NAMING_NAME_SIZE*/; ++i)
	{
		name[count++] = path[i];

		if (path[i] == LOADER_PATH_SEPARATOR)
		{
			count = 0;
		}
		else if (path[i] == '.')
		{
			if (count > 0)
			{
				last = count - 1;
			}
			else
			{
				last = 0;
			}
		}
	}

	name[last] = '\0';

	return last + 1;
}

size_t loader_path_get_extension(const loader_naming_path path, loader_naming_tag extension)
{
	size_t i, count;

	for (i = 0, count = 0; path[i] != '\0' &&
		i < LOADER_NAMING_PATH_SIZE /*&& count < LOADER_NAMING_TAG_SIZE*/; ++i)
	{
		extension[count++] = path[i];

		if (path[i] == '.')
		{
			count = 0;
		}
	}

	extension[count] = '\0';

	return count + 1;
}

size_t loader_path_get_path(const loader_naming_path path, loader_naming_path absolute)
{
	size_t i, last;

	for (i = 0, last = 0; path[i] != '\0' &&
		i < LOADER_NAMING_PATH_SIZE /*&& count < LOADER_NAMING_NAME_SIZE*/; ++i)
	{
		absolute[i] = path[i];

		if (path[i] == LOADER_PATH_SEPARATOR)
		{
			last = i + 1;
		}
	}

	absolute[last] = '\0';

	return last + 1;
}

int loader_path_is_absolute(const loader_naming_path path)
{
	#if defined(_WIN32)
		return !((path[0] != '\0' && (path[0] >= 'A' && path[0] <= 'Z')) &&
			(path[1] != '\0' && path[1] == ':') &&
			(path[2] != '\0' && path[2] == '\\'));
	#elif defined(unix) || defined(__unix__) || defined(__unix) || \
		defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
		defined(__CYGWIN__) || defined(__CYGWIN32__) || \
		(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
		return !(path[0] != '\0' && path[0] == '/');
	#else
	#	error "Unknown loader path separator"
	#endif
}
