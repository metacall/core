/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
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

#include <dynlink/dynlink.h>

#include <dynlink/dynlink_impl.h>

#include <log/log.h>

#include <string.h>

#include <windows.h>

/* -- Methods -- */

const char *dynlink_impl_interface_prefix_win32(void)
{
#if defined(__MINGW32__) || defined(__MINGW64__)
	static const char prefix_win32[] = "lib";
#else
	static const char prefix_win32[] = "";
#endif

	return prefix_win32;
}

const char *dynlink_impl_interface_extension_win32(void)
{
	static const char extension_win32[] = "dll";

	return extension_win32;
}

dynlink_impl dynlink_impl_interface_load_win32(dynlink handle)
{
	HMODULE impl;
	dynlink_flags flags = dynlink_get_flags(handle);

	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_SELF))
	{
		impl = GetModuleHandle(NULL);
	}
	else
	{
		impl = LoadLibrary(dynlink_get_path(handle));
	}

	if (impl == NULL)
	{
		DWORD error_id = GetLastError();
		LPSTR message_buffer;

		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, error_id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message_buffer, 0, NULL);

		log_write("metacall", LOG_LEVEL_ERROR, "Failed to load: %s with error code [%d]: %.*s", dynlink_get_path(handle), error_id, size - 1, (const char *)message_buffer);

		LocalFree(message_buffer);

		return NULL;
	}

	return (dynlink_impl)impl;
}

int dynlink_impl_interface_symbol_win32(dynlink handle, dynlink_impl impl, const char *name, dynlink_symbol_addr *addr)
{
	FARPROC proc_addr = GetProcAddress(impl, name);

	(void)handle;

	dynlink_symbol_cast(FARPROC, proc_addr, *addr);

	return (*addr == NULL);
}

int dynlink_impl_interface_unload_win32(dynlink handle, dynlink_impl impl)
{
	dynlink_flags flags = dynlink_get_flags(handle);

	(void)handle;

	/* Skip unlink when using global handle for loading symbols of the current process */
	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_SELF))
	{
		return 0;
	}

#if defined(__MEMORYCHECK__) || defined(__ADDRESS_SANITIZER__) || defined(__THREAD_SANITIZER__) || defined(__MEMORY_SANITIZER__)
	/* Disable dlclose when running with address sanitizer in order to maintain stacktraces */
	(void)impl;
	return 0;
#else
	return (FreeLibrary(impl) == FALSE);
#endif
}

dynlink_impl_interface dynlink_impl_interface_singleton(void)
{
	static struct dynlink_impl_interface_type impl_interface_win32 = {
		&dynlink_impl_interface_prefix_win32,
		&dynlink_impl_interface_extension_win32,
		&dynlink_impl_interface_load_win32,
		&dynlink_impl_interface_symbol_win32,
		&dynlink_impl_interface_unload_win32,
	};

	return &impl_interface_win32;
}
