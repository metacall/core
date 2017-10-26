/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <serial/serial_singleton.h>

#include <environment/environment_variable_path.h>

#include <adt/adt_set.h>

#include <log/log.h>

#include <string.h>

/* -- Definitions -- */

#define SERIAL_LIBRARY_PATH			"SERIAL_LIBRARY_PATH"
#define SERIAL_DEFAULT_LIBRARY_PATH	"serials"

/* -- Member Data -- */

struct serial_singleton_type
{
	set serials;
	char * library_path;
};

/* -- Private Methods -- */

/**
*  @brief
*    Wrapper of serial singleton instance
*
*  @return
*    Pointer to serial singleton instance
*
*/
static serial_singleton serial_singleton_instance(void);

/**
*  @brief
*    Serial singleton destroy callback iterator
*
*  @param[in] s
*    Pointer to serials set
*
*  @param[in] key
*    Pointer to current serial key
*
*  @param[in] val
*    Pointer to current serial instance
*
*  @param[in] args
*    Pointer to user defined callback arguments
*
*  @return
*    Returns zero to continue iteration, distinct from zero otherwise
*
*/
static int serial_singleton_destroy_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

/* -- Methods -- */

serial_singleton serial_singleton_instance()
{
	static struct serial_singleton_type singleton =
	{
		NULL,
		NULL
	};

	return &singleton;
}

int serial_singleton_initialize()
{
	serial_singleton singleton = serial_singleton_instance();

	if (singleton->serials == NULL)
	{
		singleton->serials = set_create(&hash_callback_str, &comparable_callback_str);

		if (singleton->serials == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial singleton set initialization");

			serial_singleton_destroy();

			return 1;
		}
	}

	if (singleton->library_path == NULL)
	{
		static const char serial_library_path[] = SERIAL_LIBRARY_PATH;

		singleton->library_path = environment_variable_path_create(serial_library_path, SERIAL_DEFAULT_LIBRARY_PATH);

		if (singleton->library_path == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial singleton library path initialization");

			serial_singleton_destroy();

			return 1;
		}
	}

	return 0;
}

int serial_singleton_register(serial s)
{
	serial_singleton singleton = serial_singleton_instance();

	const char * name = serial_name(s);

	if (set_get(singleton->serials, (const set_key)name) != NULL)
	{
		return 1;
	}

	return set_insert(singleton->serials, (const set_key)name, s);
}

serial serial_singleton_get(const char * name)
{
	serial_singleton singleton = serial_singleton_instance();

	return set_get(singleton->serials, (const set_key)name);
}

const char * serial_singleton_path()
{
	serial_singleton singleton = serial_singleton_instance();

	return singleton->library_path;
}

int serial_singleton_clear(serial s)
{
	serial_singleton singleton = serial_singleton_instance();

	const char * name = serial_name(s);

	if (set_get(singleton->serials, (const set_key)name) == NULL)
	{
		return 0;
	}

	if (set_remove(singleton->serials, (const set_key)name) == NULL)
	{
		return 1;
	}

	return 0;
}

int serial_singleton_destroy_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	(void)s;
	(void)key;
	(void)args;

	if (val != NULL)
	{
		serial_clear((serial)val);
	}

	return 0;
}

void serial_singleton_destroy()
{
	serial_singleton singleton = serial_singleton_instance();

	if (singleton->serials != NULL)
	{
		set_iterate(singleton->serials, &serial_singleton_destroy_cb_iterate, NULL);

		set_destroy(singleton->serials);

		singleton->serials = NULL;
	}

	if (singleton->library_path != NULL)
	{
		environment_variable_path_destroy(singleton->library_path);

		singleton->library_path = NULL;
	}
}
