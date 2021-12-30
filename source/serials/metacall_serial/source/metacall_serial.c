/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <metacall/metacall_version.h>

#include <metacall_serial/metacall_serial.h>
#include <metacall_serial/metacall_serial_impl.h>

/* -- Methods -- */

serial_interface metacall_serial_impl_interface_singleton(void)
{
	static struct serial_interface_type interface_instance_metacall = {
		&metacall_serial_impl_extension,
		&metacall_serial_impl_initialize,
		&metacall_serial_impl_serialize,
		&metacall_serial_impl_deserialize,
		&metacall_serial_impl_destroy
	};

	return &interface_instance_metacall;
}

const char *metacall_serial_print_info(void)
{
	static const char metacall_serial_info[] =
		"MetaCall Native Format Serial Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef METACALL_SERIAL_STATIC_DEFINE
		"Compiled as static library type\n"
#else
		"Compiled as shared library type\n"
#endif

		"\n";

	return metacall_serial_info;
}
