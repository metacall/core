/********************************************************************************/
/*	Dynamic Link by Parra Studios												*/
/*	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																				*/
/*	A library for dynamic loading and linking shared objects at run-time.		*/
/*																				*/
/********************************************************************************/

/* -- Headers -- */

#include <dynlink/dynlink.h>

#include <dynlink/dynlink_impl.h>
#include <dynlink/dynlink_impl_interface.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* -- Methods -- */

const char * dynlink_impl_interface_extension_win32(void)
{
	static const char extension_win32[0x05] = ".dll";

	return extension_win32;
}

dynlink_impl dynlink_impl_interface_load_win32(dynlink handle)
{
	HANDLE impl = LoadLibrary(dynlink_get_name(handle));

	if (impl != NULL)
	{
		return (dynlink_impl)impl;
	}

	return NULL;
}

int dynlink_impl_interface_symbol_win32(dynlink handle, dynlink_impl impl, dynlink_symbol_name name, dynlink_symbol_addr * addr)
{
	FARPROC proc_addr = GetProcAddress(impl, name);

	(void)handle;

	*addr = (dynlink_symbol_addr)proc_addr;

	return (*addr != NULL) ? 0 : 1;
}

int dynlink_impl_interface_unload_win32(dynlink handle, dynlink_impl impl)
{
	(void)handle;

	return (FreeLibrary(impl) == TRUE) ? 0 : 1;
}

dynlink_impl_interface dynlink_impl_interface_singleton_win32(void)
{
	static struct dynlink_impl_interface_type impl_interface_win32 =
	{
		&dynlink_impl_interface_extension_win32,
		&dynlink_impl_interface_load_win32,
		&dynlink_impl_interface_symbol_win32,
		&dynlink_impl_interface_unload_win32
	};

	return &impl_interface_win32;
}
