/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading ruby code at run-time into a process.
 *
 */

#include <metacall/metacall-version.h>

#include <rb_loader/rb_loader.h>
#include <rb_loader/rb_loader_impl.h>

#include <stdio.h>

loader_impl_interface rb_loader_impl_interface_singleton(void)
{
	static struct loader_impl_interface_type loader_impl_interface_rb =
	{
		&rb_loader_impl_initialize,
		&rb_loader_impl_execution_path,
		&rb_loader_impl_load,
		&rb_loader_impl_clear,
		&rb_loader_impl_discover,
		&rb_loader_impl_destroy
	};

	return &loader_impl_interface_rb;
}

void rb_loader_print_info()
{
	printf("Ruby Loader Plugin " METACALL_VERSION "\n");
	printf("Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n");

	#ifdef RB_LOADER_STATIC_DEFINE
		printf("Compiled as static library type\n");
	#else
		printf("Compiled as shared library type\n");
	#endif
}
