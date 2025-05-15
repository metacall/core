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

#include <portability/portability_library_path.h>

#include <stddef.h>
#include <string.h>

static int portability_library_path_ends_with(const char path[], const char name[]);

#if defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__FreeBSD__)

	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif
	#ifndef __USE_GNU
		#define __USE_GNU
	#endif

	#include <link.h>

#elif (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)

	#include <mach-o/dyld.h>

#elif defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)

	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <psapi.h>

#else
	#error "Unsupported platform for portability_library_path"
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

#if defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__FreeBSD__)

/* -- Type Definitions -- */

typedef struct portability_library_path_find_phdr_type *portability_library_path_find_phdr;
typedef struct portability_library_path_list_phdr_type *portability_library_path_list_phdr;

/* -- Member Data -- */

struct portability_library_path_find_phdr_type
{
	const char *name;
	char *path;
	size_t length;
};

struct portability_library_path_list_phdr_type
{
	portability_library_path_list_cb callback;
	void *data;
};

/* -- Private Methods -- */

static int portability_library_path_find_phdr_callback(struct dl_phdr_info *info, size_t size, void *data)
{
	portability_library_path_find_phdr find_phdr = (portability_library_path_find_phdr)data;

	(void)size;

	if (portability_library_path_ends_with(info->dlpi_name, find_phdr->name) == 0)
	{
		find_phdr->length = strnlen(info->dlpi_name, PORTABILITY_PATH_SIZE);

		memcpy(find_phdr->path, info->dlpi_name, sizeof(char) * (find_phdr->length + 1));

		return 1;
	}

	return 0;
}

static int portability_library_path_list_phdr_callback(struct dl_phdr_info *info, size_t size, void *data)
{
	portability_library_path_list_phdr list_phdr = (portability_library_path_list_phdr)data;

	(void)size;

	return list_phdr->callback(info->dlpi_name, list_phdr->data);
}

#endif

int portability_library_path_find(const char name[], portability_library_path_str path, size_t *length)
{
#if defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__FreeBSD__)

	struct portability_library_path_find_phdr_type data = {
		name,
		path,
		0
	};

	if (dl_iterate_phdr(&portability_library_path_find_phdr_callback, (void *)&data) != 1)
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
	DWORD modules_size;

	if (EnumProcessModules(handle_process, handle_modules, sizeof(handle_modules), &modules_size))
	{
		size_t iterator, size = modules_size / sizeof(HMODULE);

		for (iterator = 0; iterator < size; ++iterator)
		{
			if (GetModuleFileNameEx(handle_process, handle_modules[iterator], path, PORTABILITY_PATH_SIZE))
			{
				if (portability_library_path_ends_with(path, name) == 0)
				{
					if (length != NULL)
					{
						*length = strnlen(path, PORTABILITY_PATH_SIZE);
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
	size_t name_length = strnlen(name, PORTABILITY_PATH_SIZE);
	size_t name_dylib_length = name_length + 3;

	if (portability_library_path_ends_with(name, "so") == 0 && name_dylib_length < PORTABILITY_PATH_SIZE)
	{
		memcpy(path, name, sizeof(char) * (name_length - 2));
		memcpy(path, dylib_suffix, sizeof(dylib_suffix));
	}

	/* Start from 1 so we avoid the executable itself */
	for (image_index = 1; image_index < size; ++image_index)
	{
		const char *image_name = _dyld_get_image_name(image_index);

		if (portability_library_path_ends_with(image_name, path) == 0)
		{
			size_t image_length = strnlen(image_name, PORTABILITY_PATH_SIZE);

			if (image_length >= PORTABILITY_PATH_SIZE)
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
	#error "Unsupported platform for portability_library_path"
#endif
}

int portability_library_path_list(portability_library_path_list_cb callback, void *data)
{
	if (callback == NULL)
	{
		return 1;
	}

#if defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__FreeBSD__)
	{
		struct portability_library_path_list_phdr_type list_phdr = {
			callback,
			data
		};

		return dl_iterate_phdr(&portability_library_path_list_phdr_callback, (void *)&list_phdr);
	}
#elif (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
	{
		uint32_t iterator, size = _dyld_image_count();

		/* Start from 1 so we avoid the executable itself */
		for (iterator = 1; iterator < size; ++iterator)
		{
			const char *image_name = _dyld_get_image_name(iterator);

			if (callback(image_name, data) != 0)
			{
				return 1;
			}
		}

		return 0;
	}
#elif defined(WIN32) || defined(_WIN32)
	{
		HANDLE process = GetCurrentProcess();
		HMODULE modules[1024];
		DWORD modules_size;

		if (EnumProcessModules(process, modules, sizeof(modules), &modules_size))
		{
			size_t iterator, size = modules_size / sizeof(HMODULE);
			char module_name[MAX_PATH];

			for (iterator = 0; iterator < size; ++iterator)
			{
				if (GetModuleFileNameExA(process, modules[iterator], module_name, sizeof(module_name) / sizeof(char)))
				{
					if (callback(module_name, data) != 0)
					{
						return 1;
					}
				}
			}
		}

		return 0;
	}
#else
	#error "Unsupported platform for portability_library_path"
#endif
}
