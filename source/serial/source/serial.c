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

#include <environment/environment_variable_path.h>

#include <log/log.h>

#include <string.h>

/* -- Definitions -- */

#define SERIAL_LIBRARY_PATH			"SERIAL_LIBRARY_PATH"
#define SERIAL_DEFAULT_LIBRARY_PATH	"serials"

/* -- Member Data -- */

struct serial_type
{
	char * name;
	serial_impl impl;
};

/* -- Methods -- */

int serial_initialize()
{
	static const char serial_library_path[] = SERIAL_LIBRARY_PATH;

	char * library_path = environment_variable_path_create(serial_library_path, SERIAL_DEFAULT_LIBRARY_PATH);

	if (serial_singleton_initialize(library_path) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial singleton initialization");

		return 1;
	}

	return 0;
}

serial serial_create(const char * name)
{
	serial s;

	size_t name_size;

	if (name == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial name");

		return NULL;
	}

	name_size = strlen(name) + 1;

	if (name_size <= 1)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial name length");

		return NULL;
	}

	s = malloc(sizeof(struct serial_type));

	if (s == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial allocation");

		return NULL;
	}

	s->name = malloc(sizeof(char) * name_size);

	if (s->name == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial name allocation");

		free(s);

		return NULL;
	}

	strncpy(s->name, name, name_size);

	s->impl = serial_impl_create(name, serial_singleton_path());

	if (s->impl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation creation");

		free(s->name);

		free(s);

		return NULL;
	}

	if (serial_impl_load(s->impl) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation loading");

		serial_impl_destroy(s->impl);

		free(s->name);

		free(s);

		return NULL;
	}

	return s;
}

const char * serial_extension(serial s)
{
	return serial_impl_extension(s->impl);
}

const char * serial_name(serial s)
{
	return s->name;
}

const char * serial_serialize(serial s, value v, size_t * size)
{
	if (s == NULL || v == NULL || size == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serialization arguments");

		return NULL;
	}

	return serial_impl_serialize(s->impl, v, size);
}

value serial_deserialize(serial s, const char * buffer, size_t size)
{
	if (s == NULL || buffer == NULL || size == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid deserialization arguments");

		return NULL;
	}

	return serial_impl_deserialize(s->impl, buffer, size);
}

int serial_clear(serial s)
{
	if (s != NULL)
	{
		int result = 0;

		if (s->name != NULL)
		{
			free(s->name);
		}

		if (s->impl != NULL)
		{
			if (serial_impl_unload(s->impl) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation unloading");

				result = 1;
			}

			if (serial_impl_destroy(s->impl) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation destruction");

				result = 1;
			}
		}

		free(s);

		return result;
	}

	return 0;
}

void serial_destroy()
{
	serial_singleton_destroy();
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
