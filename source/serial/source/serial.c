/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <metacall/metacall_version.h>

#include <serial/serial.h>
#include <serial/serial_interface.h>

#include <plugin/plugin.h>

#include <log/log.h>

/* -- Definitions -- */

#define SERIAL_MANAGER_NAME			"serial"
#define SERIAL_LIBRARY_PATH			"SERIAL_LIBRARY_PATH"
#define SERIAL_LIBRARY_DEFAULT_PATH "serials"

/* -- Macros -- */

#define serial_iface(s) \
	plugin_iface_type(s, serial_interface)

/* -- Member Data -- */

static plugin_manager_declare(serial_manager);

/* -- Methods -- */

int serial_initialize(void)
{
	return plugin_manager_initialize(
		&serial_manager,
		SERIAL_MANAGER_NAME,
		SERIAL_LIBRARY_PATH,
#if defined(SERIAL_LIBRARY_INSTALL_PATH)
		SERIAL_LIBRARY_INSTALL_PATH,
#else
		SERIAL_LIBRARY_DEFAULT_PATH,
#endif /* SERIAL_LIBRARY_INSTALL_PATH */
		NULL,
		NULL);
}

serial serial_create(const char *name)
{
	return plugin_manager_create(&serial_manager, name, NULL, NULL);
}

const char *serial_extension(serial s)
{
	return serial_iface(s)->extension();
}

const char *serial_name(serial s)
{
	return plugin_name(s);
}

char *serial_serialize(serial s, value v, size_t *size, memory_allocator allocator)
{
	if (s == NULL || v == NULL || size == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serialization arguments");

		return NULL;
	}

	serial_handle handle = serial_iface(s)->initialize(allocator);

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation handle initialization");

		return NULL;
	}

	char *buffer = serial_iface(s)->serialize(handle, v, size);

	if (buffer == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation handle serialization");
	}

	if (serial_iface(s)->destroy(handle) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation handle destruction");
	}

	return buffer;
}

value serial_deserialize(serial s, const char *buffer, size_t size, memory_allocator allocator)
{
	if (s == NULL || buffer == NULL || size == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid deserialization arguments");

		return NULL;
	}

	serial_handle handle = serial_iface(s)->initialize(allocator);

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation handle initialization");

		return NULL;
	}

	value v = serial_iface(s)->deserialize(handle, buffer, size);

	if (v == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation handle deserialization");
	}

	if (serial_iface(s)->destroy(handle) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation handle destruction");
	}

	return v;
}

int serial_clear(serial s)
{
	return plugin_manager_clear(&serial_manager, s);
}

void serial_destroy(void)
{
	plugin_manager_destroy(&serial_manager);
}

const char *serial_print_info(void)
{
	static const char serial_info[] =
		"Serial Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef SERIAL_STATIC_DEFINE
		"Compiled as static library type"
#else
		"Compiled as shared library type"
#endif

		"\n";

	return serial_info;
}
