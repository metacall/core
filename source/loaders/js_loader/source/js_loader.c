/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading javascript code at run-time into a process.
 *
 */

#include <metacall/metacall-version.h>

#include <js_loader/js_loader.h>
#include <js_loader/js_loader_impl.h>

loader_impl_interface js_loader_impl_interface_singleton(void)
{
	static struct loader_impl_interface_type loader_impl_interface_js =
	{
		&js_loader_impl_initialize,
		&js_loader_impl_execution_path,
		&js_loader_impl_load_from_file,
		&js_loader_impl_load_from_memory,
		&js_loader_impl_load_from_package,
		&js_loader_impl_clear,
		&js_loader_impl_discover,
		&js_loader_impl_destroy
	};

	return &loader_impl_interface_js;
}

const char * js_loader_print_info()
{
	static const char js_loader_info[] =
		"Javascript Loader Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef JS_LOADER_STATIC_DEFINE
			"Compiled as static library type\n"
		#else
			"Compiled as shared library type\n"
		#endif

		"\n";

	return js_loader_info;
}
