/*
 *	Dynamic Link Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

 /* -- Headers -- */

#include <dynlink/dynlink.h>

#include <dynlink/dynlink_impl.h>

#include <log/log.h>

#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* -- Methods -- */

const char * dynlink_impl_interface_extension_win32(void)
{
	static const char extension_win32[0x04] = "dll";

	return extension_win32;
}

void dynlink_impl_interface_get_name_win32(dynlink handle, dynlink_name_impl name_impl, size_t length)
{
	strncpy(name_impl, dynlink_get_name(handle), length);

	strncat(name_impl, ".", length);

	strncat(name_impl, dynlink_impl_extension(), length);
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

int dynlink_impl_interface_symbol_win32(dynlink handle, dynlink_impl impl, dynlink_symbol_name name, dynlink_symbol_addr * addr)
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

dynlink_impl_interface dynlink_impl_interface_singleton_win32(void)
{
	static struct dynlink_impl_interface_type impl_interface_win32 =
	{
		&dynlink_impl_interface_extension_win32,
		&dynlink_impl_interface_get_name_win32,
		&dynlink_impl_interface_load_win32,
		&dynlink_impl_interface_symbol_win32,
		&dynlink_impl_interface_unload_win32
	};

	return &impl_interface_win32;
}
