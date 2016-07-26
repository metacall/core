/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading mock code at run-time into a process.
 *
 */

#include <metacall/metacall-version.h>

#include <mock_loader/mock_loader.h>
#include <mock_loader/mock_loader_impl.h>

#include <stdio.h>

loader_impl_interface mock_loader_impl_interface_singleton(void)
{
	static struct loader_impl_interface_type loader_impl_interface_mock =
	{
		&mock_loader_impl_initialize,
		&mock_loader_impl_execution_path,
		&mock_loader_impl_load,
		&mock_loader_impl_clear,
		&mock_loader_impl_discover,
		&mock_loader_impl_destroy
	};

	return &loader_impl_interface_mock;
}

void mock_loader_print_info()
{
	printf("Mock Loader Plugin " METACALL_VERSION "\n");
	printf("Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n");

	#ifdef MOCK_LOADER_STATIC_DEFINE
		printf("Compiled as static library type\n");
	#else
		printf("Compiled as shared library type\n");
	#endif
}
