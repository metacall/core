/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading nodejs code at run-time into a process.
 *
 */

#include <metacall/metacall-version.h>

#include <node_loader/node_loader.h>
#include <node_loader/node_loader_impl.h>

loader_impl_interface node_loader_impl_interface_singleton(void)
{
	static struct loader_impl_interface_type loader_impl_interface_node =
	{
		&node_loader_impl_initialize,
		&node_loader_impl_execution_path,
		&node_loader_impl_load_from_file,
		&node_loader_impl_load_from_memory,
		&node_loader_impl_load_from_package,
		&node_loader_impl_clear,
		&node_loader_impl_discover,
		&node_loader_impl_destroy
	};

	return &loader_impl_interface_node;
}

const char * node_loader_print_info()
{
	static const char node_loader_info[] =
		"Javascript Loader Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef JS_LOADER_STATIC_DEFINE
			"Compiled as static library type\n"
		#else
			"Compiled as shared library type\n"
		#endif

		"\n";

	return node_loader_info;
}
