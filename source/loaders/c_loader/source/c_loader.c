/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading c code at run-time into a process.
 *
 */

#include <metacall/metacall_version.h>

#include <c_loader/c_loader.h>
#include <c_loader/c_loader_impl.h>

loader_impl_interface c_loader_impl_interface_singleton(void)
{
	static struct loader_impl_interface_type loader_impl_interface_c =
	{
		&c_loader_impl_initialize,
		&c_loader_impl_execution_path,
		&c_loader_impl_load,
		&c_loader_impl_clear,
		&c_loader_impl_discover,
		&c_loader_impl_destroy
	};

	return &loader_impl_interface_c;
}

const char * c_loader_print_info()
{
	static const char c_loader_info[] =
		"C Loader Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef C_LOADER_STATIC_DEFINE
			"Compiled as static library type\n"
		#else
			"Compiled as shared library type\n"
		#endif

		"\n";

	return c_loader_info;
}
