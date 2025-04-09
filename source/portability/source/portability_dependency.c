/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

#include <portability/portability_dependency.h>

#include <stddef.h>

#if defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__FreeBSD__)

	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif
	#include <link.h>

#elif (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)

	#include <mach-o/dyld.h>

#elif defined(WIN32) || defined(_WIN32)

	#include <psapi.h>
	#include <windows.h>

#else
	#error "Unsupported platform for portability_dependendency_iterate"
#endif

#if defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__FreeBSD__)

/* -- Type Definitions -- */

typedef struct portability_dependendency_iterate_phdr_type *portability_dependendency_iterate_phdr;

/* -- Member Data -- */

struct portability_dependendency_iterate_phdr_type
{
	portability_dependendency_iterate_cb callback;
	void *data;
};

/* -- Private Methods -- */

static int portability_dependendency_iterate_phdr_callback(struct dl_phdr_info *info, size_t size, void *data)
{
	portability_dependendency_iterate_phdr phdr = (portability_dependendency_iterate_phdr)data;

	(void)size;

	return phdr->callback(info->dlpi_name, phdr->data);
}

#endif

int portability_dependendency_iterate(portability_dependendency_iterate_cb callback, void *data)
{
	if (callback == NULL)
	{
		return 1;
	}

#if defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__FreeBSD__)
	{
		struct portability_dependendency_iterate_phdr_type phdr = {
			callback,
			data
		};

		return dl_iterate_phdr(&portability_dependendency_iterate_phdr_callback, (void *)&phdr);
	}
#elif (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
	{
		uint32_t iterator, size = _dyld_image_count();

		for (iterator = 0; iterator < size; ++iterator)
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

			for (iterator = 0; i < size; ++iterator)
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
#endif
}
