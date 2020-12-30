/*
*	Configuration Library by Parra Studios
*	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple configuration formats.
*
*/

/* -- Headers -- */

#include <configuration/configuration_singleton.h>
#include <configuration/configuration_object.h>
#include <configuration/configuration_impl.h>

#include <adt/adt_set.h>
#include <adt/adt_vector.h>

#include <log/log.h>

#include <string.h>

/* -- Member Data -- */

struct configuration_singleton_type
{
	set scopes;
	configuration global;
};

/* -- Private Methods -- */

static int configuration_singleton_destroy_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

/* -- Member Data -- */

static struct configuration_singleton_type configuration_singleton_default =
{
	NULL,
	NULL
};

configuration_singleton configuration_singleton_ptr = &configuration_singleton_default;

/* -- Methods -- */

configuration_singleton configuration_singleton_instance()
{
	return configuration_singleton_ptr;
}

void configuration_singleton_copy(configuration_singleton singleton)
{
	configuration_singleton_ptr = singleton;
}

int configuration_singleton_initialize(configuration global)
{
	configuration_singleton singleton = configuration_singleton_instance();

	if (singleton->scopes != NULL && singleton->global != NULL)
	{
		return 0;
	}

	singleton->scopes = set_create(&hash_callback_str, &comparable_callback_str);

	if (singleton->scopes == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration singleton scope map initialization");

		configuration_singleton_destroy();

		return 1;
	}

	singleton->global = global;

	if (set_insert(singleton->scopes, (const set_key)configuration_object_name(singleton->global), singleton->global) != 0)
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

	if (set_get(singleton->scopes, (const set_key)configuration_object_name(config)) != NULL)
	{
		return 1;
	}

	return set_insert(singleton->scopes, (const set_key)configuration_object_name(config), config);
}

configuration configuration_singleton_get(const char * name)
{
	configuration_singleton singleton = configuration_singleton_instance();

	return set_get(singleton->scopes, (const set_key)name);
}

int configuration_singleton_clear(configuration config)
{
	configuration_singleton singleton = configuration_singleton_instance();

	if (set_get(singleton->scopes, (const set_key)configuration_object_name(config)) == NULL)
	{
		return 0;
	}

	if (set_remove(singleton->scopes, (const set_key)configuration_object_name(config)) == NULL)
	{
		return 1;
	}

	return 0;
}

int configuration_singleton_destroy_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	(void)s;
	(void)key;
	(void)args;

	if (val != NULL)
	{
		configuration config = val;

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
		set_iterate(singleton->scopes, &configuration_singleton_destroy_cb_iterate, NULL);

		set_destroy(singleton->scopes);

		singleton->scopes = NULL;
	}
}
