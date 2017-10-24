/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <serial/serial_singleton.h>

#include <adt/adt_hash_map.h>

#include <log/log.h>

#include <string.h>

/* -- Member Data -- */

struct serial_singleton_type
{
	hash_map serials;
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
*  @param[in] map
*    Pointer to serials map
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
static int serial_singleton_destroy_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args);

/* -- Methods -- */

serial_singleton serial_singleton_instance()
{
	static struct serial_singleton_type singleton =
	{
		NULL
	};

	return &singleton;
}

int serial_singleton_initialize()
{
	serial_singleton singleton = serial_singleton_instance();

	if (singleton->serials != NULL)
	{
		return 0;
	}

	singleton->serials = hash_map_create(&hash_callback_str, &comparable_callback_str);

	if (singleton->serials == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial singleton scope map initialization");

		serial_singleton_destroy();

		return 1;
	}

	return 0;
}

int serial_singleton_register(serial s)
{
	serial_singleton singleton = serial_singleton_instance();

	const char * name = serial_name(s);

	if (hash_map_get(singleton->serials, (const hash_map_key)name) != NULL)
	{
		return 1;
	}

	return hash_map_insert(singleton->serials, (const hash_map_key)name, s);
}

serial serial_singleton_get(const char * name)
{
	serial_singleton singleton = serial_singleton_instance();

	return hash_map_get(singleton->serials, (const hash_map_key)name);
}

int serial_singleton_clear(serial s)
{
	serial_singleton singleton = serial_singleton_instance();

	const char * name = serial_name(s);

	if (hash_map_get(singleton->serials, (const hash_map_key)name) == NULL)
	{
		return 0;
	}

	if (hash_map_remove(singleton->serials, (const hash_map_key)name) == NULL)
	{
		return 1;
	}

	return 0;
}

int serial_singleton_destroy_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args)
{
	(void)map;
	(void)key;
	(void)args;

	if (val != NULL)
	{
		serial s = val;

		serial_clear(s);
	}

	return 0;
}

void serial_singleton_destroy()
{
	serial_singleton singleton = serial_singleton_instance();

	if (singleton->serials != NULL)
	{
		hash_map_iterate(singleton->serials, &serial_singleton_destroy_cb_iterate, NULL);

		hash_map_destroy(singleton->serials);

		singleton->serials = NULL;
	}
}
