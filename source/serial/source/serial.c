/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <metacall/metacall_version.h>

#include <serial/serial.h>
#include <serial/serial_impl.h>
#include <serial/serial_singleton.h>

#include <log/log.h>

#include <string.h>

/* -- Member Data -- */

struct serial_type
{
	char *name;
	serial_impl impl;
};

/* -- Methods -- */

int serial_initialize()
{
	if (serial_singleton_initialize() != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial singleton initialization");

		return 1;
	}

	return 0;
}

serial serial_create(const char *name)
{
	serial s;

	size_t name_length;

	if (name == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial name");

		return NULL;
	}

	s = serial_singleton_get(name);

	if (s != NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "Serial <%p> already exists", (void *)s);

		return s;
	}

	name_length = strlen(name);

	if (name_length == 0)
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

	s->name = malloc(sizeof(char) * (name_length + 1));

	if (s->name == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial name allocation");

		free(s);

		return NULL;
	}

	strncpy(s->name, name, name_length);

	s->name[name_length] = '\0';

	s->impl = serial_impl_create();

	if (s->impl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation creation");

		free(s->name);

		free(s);

		return NULL;
	}

	if (serial_impl_load(s->impl, serial_singleton_path(), s->name) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation loading");

		serial_impl_destroy(s->impl);

		free(s->name);

		free(s);

		return NULL;
	}

	if (serial_singleton_register(s) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial singleton insert");

		serial_impl_destroy(s->impl);

		free(s->name);

		free(s);

		return NULL;
	}

	return s;
}

const char *serial_extension(serial s)
{
	return serial_impl_extension(s->impl);
}

const char *serial_name(serial s)
{
	return s->name;
}

char *serial_serialize(serial s, value v, size_t *size, memory_allocator allocator)
{
	if (s == NULL || v == NULL || size == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serialization arguments");

		return NULL;
	}

	return serial_impl_serialize(s->impl, v, size, allocator);
}

value serial_deserialize(serial s, const char *buffer, size_t size, memory_allocator allocator)
{
	if (s == NULL || buffer == NULL || size == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid deserialization arguments");

		return NULL;
	}

	return serial_impl_deserialize(s->impl, buffer, size, allocator);
}

int serial_clear(serial s)
{
	if (s != NULL)
	{
		int result = 0;

		if (serial_singleton_clear(s) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial singleton clear");

			result = 1;
		}

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

const char *serial_print_info()
{
	static const char serial_info[] =
		"Serial Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef SERIAL_STATIC_DEFINE
		"Compiled as static library type"
#else
		"Compiled as shared library type"
#endif

		"\n";

	return serial_info;
}
