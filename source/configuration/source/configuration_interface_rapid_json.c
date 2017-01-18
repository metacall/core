/*
*	Configuration Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple configuration formats.
*
*/

/* -- Headers -- */

#include <configuration/configuration_interface_rapid_json.h>

/* -- Methods -- */

configuration_interface configuration_interface_instance_rapid_json()
{
	static struct configuration_interface_type interface_instance_rapid_json =
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	};

	return &interface_instance_rapid_json;
}
