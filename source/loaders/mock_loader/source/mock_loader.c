/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading mock code at run-time into a process.
 *
 */

#include <metacall/metacall_version.h>

#include <mock_loader/mock_loader.h>
#include <mock_loader/mock_loader_impl.h>

loader_impl_interface mock_loader_impl_interface_singleton()
{
	static struct loader_impl_interface_type loader_impl_interface_mock =
	{
		&mock_loader_impl_initialize,
		&mock_loader_impl_execution_path,
		&mock_loader_impl_load_from_file,
		&mock_loader_impl_load_from_memory,
		&mock_loader_impl_load_from_package,
		&mock_loader_impl_clear,
		&mock_loader_impl_discover,
		&mock_loader_impl_destroy
	};

	return &loader_impl_interface_mock;
}

const char * mock_loader_print_info()
{
	static const char mock_loader_info[] =
		"Mock Loader Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef MOCK_LOADER_STATIC_DEFINE
			"Compiled as static library type\n"
		#else
			"Compiled as shared library type\n"
		#endif

		"\n";

	return mock_loader_info;
}
