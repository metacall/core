/*
*	Configuration Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple configuration formats.
*
*/

/* -- Headers -- */

#include <configuration/configuration_interface_rapid_json.h>
#include <configuration/configuration_impl_rapid_json.h>

/* -- Methods -- */

configuration_interface configuration_interface_instance_rapid_json()
{
	static struct configuration_interface_type interface_instance_rapid_json =
	{
		&configuration_impl_rapid_json_extension,
		&configuration_impl_rapid_json_initialize,
		&configuration_impl_rapid_json_load,
		&configuration_impl_rapid_json_unload,
		&configuration_impl_rapid_json_destroy
	};

	return &interface_instance_rapid_json;
}
