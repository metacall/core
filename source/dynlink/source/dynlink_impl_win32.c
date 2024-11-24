/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

/* -- Methods -- */

const char *dynlink_impl_interface_extension_win32(void)
{
	static const char extension_win32[] = "dll";

	return extension_win32;
}

void dynlink_impl_interface_get_name_win32(dynlink_name name, dynlink_name_impl name_impl, size_t size)
{
	strncpy(name_impl, name, size);

	strncat(name_impl, ".", size - 1);

	strncat(name_impl, dynlink_impl_extension(), size - 1);
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

#if defined(__MINGW32__) || defined(__MINGW64__)
	// For MinGW, set the symbol member of the struct
	(*addr)->symbol = (dynlink_symbol_addr_win32_impl)proc_addr;
#else
	// For MSVC or other compilers, cast directly
	*addr = (dynlink_symbol_addr)proc_addr;
#endif
	return (*addr == NULL);
}

int dynlink_impl_interface_unload_win32(dynlink handle, dynlink_impl impl)
{
	(void)handle;

	return (FreeLibrary(impl) == FALSE);
}

dynlink_impl_interface dynlink_impl_interface_singleton(void)
{
	static struct dynlink_impl_interface_type impl_interface_win32 = {
		&dynlink_impl_interface_extension_win32,
		&dynlink_impl_interface_get_name_win32,
		&dynlink_impl_interface_load_win32,
		&dynlink_impl_interface_symbol_win32,
		&dynlink_impl_interface_unload_win32,
	};

	return &impl_interface_win32;
}
