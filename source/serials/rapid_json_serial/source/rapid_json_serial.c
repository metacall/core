/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <metacall/metacall_version.h>

#include <rapid_json_serial/rapid_json_serial.h>
#include <rapid_json_serial/rapid_json_serial_impl.h>

/* -- Methods -- */

serial_interface rapid_json_serial_impl_interface_singleton(void)
{
	static struct serial_interface_type interface_instance_rapid_json = {
		&rapid_json_serial_impl_extension,
		&rapid_json_serial_impl_initialize,
		&rapid_json_serial_impl_serialize,
		&rapid_json_serial_impl_deserialize,
		&rapid_json_serial_impl_destroy
	};

	return &interface_instance_rapid_json;
}

const char *rapid_json_serial_print_info(void)
{
	static const char rapid_json_serial_info[] =
		"Rapid JSON Serial Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef RAPID_JSON_SERIAL_STATIC_DEFINE
		"Compiled as static library type\n"
#else
		"Compiled as shared library type\n"
#endif

		"\n";

	return rapid_json_serial_info;
}
