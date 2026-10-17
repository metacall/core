/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <metacall/metacall_version.h>

#include <simd_json_serial/simd_json_serial.h>
#include <simd_json_serial/simd_json_serial_impl.h>

/* -- Methods -- */

serial_interface simd_json_serial_impl_interface_singleton(void)
{
	static struct serial_interface_type interface_instance_simd_json = {
		&simd_json_serial_impl_extension,
		&simd_json_serial_impl_initialize,
		&simd_json_serial_impl_serialize,
		&simd_json_serial_impl_deserialize,
		&simd_json_serial_impl_destroy
	};

	return &interface_instance_simd_json;
}

const char *simd_json_serial_print_info(void)
{
	static const char simd_json_serial_info[] =
		"Simd JSON Serial Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef SIMD_JSON_SERIAL_STATIC_DEFINE
		"Compiled as static library type\n"
#else
		"Compiled as shared library type\n"
#endif

		"\n";

	return simd_json_serial_info;
}
