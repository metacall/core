/********************************************************************************/
/*	Dynamic Link by Parra Studios												*/
/*	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																				*/
/*	A library for dynamic loading and linking shared objects at run-time.		*/
/*																				*/
/********************************************************************************/

/* -- Headers -- */

#include <dynlink/dynlink_impl.h>

#include <dynlink/dynlink_impl_interface.h>

#include <dynlink/dynlink_impl_macos.h>
#include <dynlink/dynlink_impl_unix.h>
#include <dynlink/dynlink_impl_win32.h>

/* -- Methods -- */

dynlink_impl_interface_singleton dynlink_interface(void)
{
	#if defined(__APPLE__)
		return &dynlink_impl_interface_singleton_macos;
	#elif defined(UNIX)
		return &dynlink_impl_interface_singleton_unix;
	#elif defined (WIN32)
		return &dynlink_impl_interface_singleton_win32;
	#else
		return &dynlink_impl_interface_singleton_null;
	#endif
}

const char * dynlink_impl_extension(void)
{
	dynlink_impl_interface_singleton singleton = dynlink_interface();

	return singleton()->extension();
}

dynlink_impl dynlink_impl_load(dynlink handle)
{
	dynlink_impl_interface_singleton singleton = dynlink_interface();

	return singleton()->load(handle);
}

int dynlink_impl_symbol(dynlink handle, dynlink_impl impl, dynlink_symbol_name symbol_name, dynlink_symbol_addr * symbol_address)
{
	dynlink_impl_interface_singleton singleton = dynlink_interface();

	return singleton()->symbol(handle, impl, symbol_name, symbol_address);
}

void dynlink_impl_unload(dynlink handle, dynlink_impl impl)
{
	dynlink_impl_interface_singleton singleton = dynlink_interface();

	singleton()->unload(handle, impl);
}
