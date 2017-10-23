/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <metacall/metacall-version.h>

#include <serial/serial.h>
#include <serial/serial_singleton.h>
#include <serial/serial_impl.h>

#include <environment/environment_variable.h>
#include <environment/environment_variable_path.h>

#include <log/log.h>

/* -- Methods -- */

int serial_initialize()
{
	return 1;
}

serial serial_create(const char * name)
{
	return NULL;
}

const char * serial_serialize(serial s, value v, size_t * size)
{
	return NULL;
}

value serial_deserialize(serial s, const char * buffer, size_t size)
{
	return NULL;
}

int serial_clear(serial s)
{
	int result = serial_impl_unload(s);

	result |= serial_singleton_clear(s);

	return result;
}

void serial_destroy()
{
	serial_singleton_destroy();

	serial_impl_destroy();
}

const char * serial_print_info()
{
	static const char serial_info[] =
		"Serial Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef SERIAL_STATIC_DEFINE
			"Compiled as static library type"
		#else
			"Compiled as shared library type"
		#endif

		"\n";

	return serial_info;
}
