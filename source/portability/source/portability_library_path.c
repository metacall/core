/*
 *	Portability Library by Parra Studios
 *	A generic cross-platform portability utility.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <portability/portability_library_path.h>

#include <string.h>

#define PORTABILITY_LIBRARY_PATH_SIZE (sizeof(portability_library_path_str) / sizeof(char))

static int portability_library_path_ends_with(const char path[], const char name[]);

#if defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux)

	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif
	#ifndef __USE_GNU
		#define __USE_GNU
	#endif

	#include <link.h>

struct phdr_callback_type
{
	const char *name;
	char *path;
	size_t length;
};

static int portability_library_path_phdr_callback(struct dl_phdr_info *info, size_t size, void *data)
{
	struct phdr_callback_type *cb = (struct phdr_callback_type *)data;

	(void)size;

	if (portability_library_path_ends_with(info->dlpi_name, cb->name) == 0)
	{
		cb->length = strnlen(info->dlpi_name, PORTABILITY_LIBRARY_PATH_SIZE);

		if (cb->length >= PORTABILITY_LIBRARY_PATH_SIZE)
		{
			return 2;
		}

		memcpy(cb->path, info->dlpi_name, sizeof(char) * (cb->length + 1));

		return 1;
	}

	return 0;
}

#elif defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)

	#define WIN32_LEAN_AND_MEAN
	#include <psapi.h>
	#include <windows.h>

#elif (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)

	#include <mach-o/dyld.h>

#endif

int portability_library_path_ends_with(const char path[], const char name[])
{
	if (path == NULL || name == NULL)
	{
		return 1;
	}

	size_t path_length = strlen(path);
	size_t name_length = strlen(name);

	return !(name_length <= path_length && strncmp(path + path_length - name_length, name, name_length) == 0);
}

int portability_library_path(const char name[], portability_library_path_str path, size_t *length)
{
#if defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux)

	struct phdr_callback_type data = {
		name,
		path,
		0
	};

	if (dl_iterate_phdr(&portability_library_path_phdr_callback, (void *)&data) != 1)
	{
		return 1;
	}

	if (length != NULL)
	{
		*length = data.length;
	}

	return 0;

#elif defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)

	HMODULE handle_modules[1024];
	HANDLE handle_process = GetCurrentProcess();
	DWORD cb_needed;

	if (EnumProcessModules(handle_process, handle_modules, sizeof(handle_modules), &cb_needed))
	{
		size_t iterator;

		for (iterator = 0; iterator < (cb_needed / sizeof(HMODULE)); ++iterator)
		{
			if (GetModuleFileNameEx(handle_process, handle_modules[iterator], path, PORTABILITY_LIBRARY_PATH_SIZE))
			{
				if (portability_library_path_ends_with(path, name) == 0)
				{
					if (length != NULL)
					{
						*length = strnlen(path, PORTABILITY_LIBRARY_PATH_SIZE);
					}

					return 0;
				}
			}
		}
	}

	return 1;

#elif (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)

	static const char dylib_suffix[] = "dylib";
	uint32_t image_index, size = _dyld_image_count();
	size_t name_length = strnlen(name, PORTABILITY_LIBRARY_PATH_SIZE);
	size_t name_dylib_length = name_length + 3;

	if (portability_library_path_ends_with(name, "so") == 0 && name_dylib_length < PORTABILITY_LIBRARY_PATH_SIZE)
	{
		memcpy(path, name, sizeof(char) * (name_length - 2));
		memcpy(path, dylib_suffix, sizeof(dylib_suffix));
	}

	for (image_index = 0; image_index < size; ++image_index)
	{
		const char *image_name = _dyld_get_image_name(image_index);

		if (portability_library_path_ends_with(image_name, path) == 0)
		{
			size_t image_length = strnlen(image_name, PORTABILITY_LIBRARY_PATH_SIZE);

			if (image_length >= PORTABILITY_LIBRARY_PATH_SIZE)
			{
				return 1;
			}

			memcpy(path, image_name, sizeof(char) * (image_length + 1));

			if (length != NULL)
			{
				*length = image_length;
			}

			return 0;
		}
	}

	return 1;

#else
	/* Not supported */
	return 1;
#endif
}
