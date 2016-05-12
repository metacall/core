/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading python code at run-time into a process.
 *
 */

#include <metacall/metacall-version.h>

#include <py_loader/py_loader.h>

#include <py_loader/py_loader_impl.h>

#include <stdio.h>

loader_impl_interface py_loader_impl_interface_singleton(void)
{
	static struct loader_impl_interface_type loader_impl_interface_py =
	{
		&py_loader_impl_initialize,
		&py_loader_impl_execution_path,
		&py_loader_impl_load,
		&py_loader_impl_clear,
		&py_loader_impl_discover,
		&py_loader_impl_destroy
	};

	return &loader_impl_interface_py;
}

void py_loader_print_info()
{
	printf("Python Loader Plugin " METACALL_VERSION "\n");
	printf("Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n");

	#ifdef PY_LOADER_STATIC_DEFINE
		printf("Compiled as static library type\n");
	#else
		printf("Compiled as shared library type\n");
	#endif
}
