/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
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

/* -- Headers -- */

#include <dynlink/dynlink.h>

#include <dynlink/dynlink_impl.h>

#include <log/log.h>

#include <string.h>

#include <windows.h>

#include <psapi.h>

/* -- Methods -- */

const char *dynlink_impl_interface_extension_win32(void)
{
	static const char extension_win32[] = "dll";

	return extension_win32;
}

static void dynlink_impl_interface_get_name_str_win32(dynlink_name name, dynlink_name_impl name_impl, size_t size)
{
	strncat(name_impl, name, size);

	strncat(name_impl, ".", size);

	strncat(name_impl, dynlink_impl_extension(), size);
}

void dynlink_impl_interface_get_name_win32(dynlink handle, dynlink_name_impl name_impl, size_t size)
{
	strncpy(name_impl, dynlink_get_name(handle), size);

	strncat(name_impl, ".", size);

	strncat(name_impl, dynlink_impl_extension(), size);
}

dynlink_impl dynlink_impl_interface_load_win32(dynlink handle)
{
	HANDLE impl = LoadLibrary(dynlink_get_name_impl(handle));

	if (impl == NULL)
	{
		DWORD error_id = GetLastError();
		LPSTR message_buffer;

		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, error_id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message_buffer, 0, NULL);

		log_write("metacall", LOG_LEVEL_ERROR, "Failed to load: %s with error code [%d]: %.*s", dynlink_get_name_impl(handle), error_id, size - 1, (const char *)message_buffer);

		LocalFree(message_buffer);

		return NULL;
	}

	return (dynlink_impl)impl;
}

int dynlink_impl_interface_symbol_win32(dynlink handle, dynlink_impl impl, dynlink_symbol_name name, dynlink_symbol_addr *addr)
{
	FARPROC proc_addr = GetProcAddress(impl, name);

	(void)handle;

	*addr = (dynlink_symbol_addr)proc_addr;

	return (*addr == NULL);
}

int dynlink_impl_interface_unload_win32(dynlink handle, dynlink_impl impl)
{
	(void)handle;

	return (FreeLibrary(impl) == FALSE);
}

static char *dynlink_impl_interface_strip_lib_path_win32(char *metacall_lib_path, char *metacall_lib_name)
{
	/* TODO: Review this */
	size_t lib_path_len = strlen(metacall_lib_path) - (strlen(metacall_lib_name) + 1);
	char *custom_lib_path = malloc(sizeof(char) * (lib_path_len + 1));
	custom_lib_path[lib_path_len] = 0;
	strncpy(custom_lib_path, metacall_lib_path, lib_path_len);
	return custom_lib_path;
}

char *dynlink_impl_interface_lib_path_win32(dynlink_name name, int (*comparator)(dynlink_path, dynlink_name))
{
	/* TODO: Review this */
	dynlink_name_impl metacall_lib_name = { 0 };
	dynlink_impl_interface_get_name_str_win32(name, metacall_lib_name, DYNLINK_NAME_IMPL_SIZE);
	HMODULE handle_modules[1024];
	HANDLE handle_process;
	DWORD cb_needed;
	unsigned int i;
	handle_process = GetCurrentProcess();
	char *metacall_lib_path = NULL;
	int found_lib_path = 0;
	if (EnumProcessModules(handle_process, handle_modules, sizeof(handle_modules), &cb_needed))
	{
		for (i = 0; i < (cb_needed / sizeof(HMODULE)); i++)
		{
			/* TODO: Review this */
			TCHAR lib_path[MAX_PATH];

			/* Get the full path to the module's file */
			if (GetModuleFileNameEx(handle_process, handle_modules[i], lib_path,
					sizeof(lib_path) / sizeof(TCHAR)))
			{
				if (comparator(lib_path, metacall_lib_name) == 0)
				{
					found_lib_path = 1;
					metacall_lib_path = dynlink_impl_interface_strip_lib_path_win32(lib_path, metacall_lib_name);
					break;
				}
			}
		}
	}
	if (!found_lib_path)
	{
		free(metacall_lib_path);
		return NULL;
	}
	return metacall_lib_path;
}

dynlink_impl_interface dynlink_impl_interface_singleton_win32(void)
{
	static struct dynlink_impl_interface_type impl_interface_win32 = {
		&dynlink_impl_interface_extension_win32,
		&dynlink_impl_interface_get_name_win32,
		&dynlink_impl_interface_load_win32,
		&dynlink_impl_interface_symbol_win32,
		&dynlink_impl_interface_unload_win32,
		&dynlink_impl_interface_lib_path_win32
	};

	return &impl_interface_win32;
}
