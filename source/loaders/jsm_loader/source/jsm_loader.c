/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading javascript code at run-time into a process.
 *
 */

#include <metacall/metacall_version.h>

#include <jsm_loader/jsm_loader.h>
#include <jsm_loader/jsm_loader_impl.h>

loader_impl_interface jsm_loader_impl_interface_singleton(void)
{
	static struct loader_impl_interface_type loader_impl_interface_jsm =
	{
		&jsm_loader_impl_initialize,
		&jsm_loader_impl_execution_path,
		&jsm_loader_impl_load,
		&jsm_loader_impl_clear,
		&jsm_loader_impl_discover,
		&jsm_loader_impl_destroy
	};

	return &loader_impl_interface_jsm;
}

const char * jsm_loader_print_info()
{
	static const char jsm_loader_info[] =
		"Javascript Loader Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef JSM_LOADER_STATIC_DEFINE
			"Compiled as static library type\n"
		#else
			"Compiled as shared library type\n"
		#endif

		"\n";

	return jsm_loader_info;
}
