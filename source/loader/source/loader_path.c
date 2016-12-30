/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#include <loader/loader_path.h>

int loader_path_get_name(const loader_naming_path path, loader_naming_name name)
{
	int i, count, last;

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
			last = count - 1;
		}
	}

	name[last] = '\0';

	return last + 1;
}

int loader_path_get_extension(const loader_naming_path path, loader_naming_extension extension)
{
	int i, count;

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
