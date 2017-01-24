/*
*	Configuration Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple configuration formats.
*
*/

/* -- Headers -- */

#include <configuration/configuration_singleton.h>
#include <configuration/configuration_object.h>
#include <configuration/configuration_impl.h>

#include <adt/adt_hash_map.h>
#include <adt/adt_vector.h>

#include <log/log.h>

#include <string.h>

/* -- Member Data -- */

struct configuration_singleton_type
{
	hash_map scopes;
	configuration global;
};

/* -- Private Methods -- */

/**
*  @brief
*    Wrapper of configuration singleton instance
*
*  @return
*    Pointer to configuration singleton instance
*
*/
static configuration_singleton configuration_singleton_instance(void);

static int configuration_singleton_destroy_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args);

/* -- Methods -- */

configuration_singleton configuration_singleton_instance()
{
	static struct configuration_singleton_type singleton =
	{
		NULL,
		NULL
	};

	return &singleton;
}

int configuration_singleton_initialize(configuration global)
{
	configuration_singleton singleton = configuration_singleton_instance();

	if (singleton->scopes != NULL && singleton->global != NULL)
	{
		return 0;
	}

	singleton->scopes = hash_map_create(&hash_callback_str, &comparable_callback_str);

	if (singleton->scopes == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration singleton scope map initialization");

		configuration_singleton_destroy();

		return 1;
	}

	singleton->global = global;

	if (hash_map_insert(singleton->scopes, (const hash_map_key)configuration_object_name(singleton->global), singleton->global) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration singleton global scope map insertion");

		configuration_singleton_destroy();

		return 1;
	}

	return 0;
}

int configuration_singleton_register(configuration config)
{
	configuration_singleton singleton = configuration_singleton_instance();

	if (hash_map_get(singleton->scopes, (const hash_map_key)configuration_object_name(config)) != NULL)
	{
		return 1;
	}

	return hash_map_insert(singleton->scopes, (const hash_map_key)configuration_object_name(config), config);
}

configuration configuration_singleton_get(const char * name)
{
	configuration_singleton singleton = configuration_singleton_instance();

	return hash_map_get(singleton->scopes, (const hash_map_key)name);
}

int configuration_singleton_destroy_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args)
{
	(void)map;
	(void)key;
	(void)args;

	if (val != NULL)
	{
		configuration config = val;

		configuration_impl_unload(config);

		configuration_object_destroy(config);
	}

	return 0;
}

void configuration_singleton_destroy()
{
	configuration_singleton singleton = configuration_singleton_instance();

	singleton->global = NULL;

	if (singleton->scopes != NULL)
	{
		hash_map_iterate(singleton->scopes, &configuration_singleton_destroy_cb_iterate, NULL);

		hash_map_destroy(singleton->scopes);

		singleton->scopes = NULL;
	}
}
