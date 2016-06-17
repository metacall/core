/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading c code at run-time into a process.
 *
 */

#include <metacall/metacall-version.h>

#include <c_loader/c_loader.h>
#include <c_loader/c_loader_impl.h>

#include <stdio.h>

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

void c_loader_print_info()
{
	printf("C Loader Plugin " METACALL_VERSION "\n");
	printf("Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n");

	#ifdef C_LOADER_STATIC_DEFINE
		printf("Compiled as static library type\n");
	#else
		printf("Compiled as shared library type\n");
	#endif
}
