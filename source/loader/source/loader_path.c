/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#include <loader/loader_path.h>

size_t loader_path_get_name(const loader_naming_path path, loader_naming_name name)
{
	size_t i, count, last;

	for (i = 0, count = 0, last = 0; path[i] != '\0' &&
		i < LOADER_NAMING_PATH_SIZE /*&& count < LOADER_NAMING_NAME_SIZE*/; ++i)
	{
		name[count++] = path[i];

		if (path[i] == '/' || path[i] == '\\')
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

size_t loader_path_get_extension(const loader_naming_path path, loader_naming_extension extension)
{
	size_t i, count;

	for (i = 0, count = 0; path[i] != '\0' &&
		i < LOADER_NAMING_PATH_SIZE /*&& count < LOADER_NAMING_EXTENSION_SIZE*/; ++i)
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

		if (path[i] == '/' || path[i] == '\\')
		{
			last = i + 1;
		}
	}

	absolute[last] = '\0';

	return last + 1;
}
