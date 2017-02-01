/*
 *	Configuration Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing json configuration format.
 *
 */

/* -- Headers -- */

#include <metacall/metacall-version.h>

#include <rapid_json_config/rapid_json_config.h>
#include <rapid_json_config/rapid_json_config_impl.h>

/* -- Methods -- */

configuration_interface rapid_json_config_impl_interface_singleton()
{
	static struct configuration_interface_type interface_instance_rapid_json =
	{
		&rapid_json_config_impl_extension,
		&rapid_json_config_impl_initialize,
		&rapid_json_config_impl_load,
		&rapid_json_config_impl_unload,
		&rapid_json_config_impl_destroy
	};

	return &interface_instance_rapid_json;
}

const char * rapid_json_config_print_info()
{
	static const char py_loader_info[] =
		"Rapid JSON Configuration Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef PY_LOADER_STATIC_DEFINE
			"Compiled as static library type\n"
		#else
			"Compiled as shared library type\n"
		#endif

		"\n";

	return py_loader_info;
}
