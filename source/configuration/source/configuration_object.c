/*
*	Configuration Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple configuration formats.
*
*/

/* -- Headers -- */

#include <configuration/configuration_object.h>
#include <configuration/configuration_stream.h>

#include <adt/adt_hash_map.h>

#include <log/log.h>

#include <string.h>

/* -- Member Data -- */

struct configuration_type
{
	char * name;
	char * path;
	hash_map map;
	configuration parent;
	char * source;
	configuration_impl impl;
};

/* -- Private Methods -- */

static int configuration_object_destroy_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args);

/* -- Methods -- */

configuration configuration_object_initialize(const char * name, const char * path, configuration parent)
{
	configuration config = malloc(sizeof(struct configuration_type));

	size_t name_size, path_size;

	if (config == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration object allocation");

		return NULL;
	}

	config->source = configuration_stream_create(path);

	if (config->source == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration file path (%s)", path);

		free(config);

		return NULL;
	}

	name_size = strlen(name) + 1;

	config->name = malloc(name_size * sizeof(char));

	path_size = strlen(path) + 1;

	config->path = malloc(path_size * sizeof(char));

	config->map = hash_map_create(&hash_callback_str, &comparable_callback_str);

	if (config->name == NULL || config->path == NULL || config->map == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration object initialization");

		configuration_object_destroy(config);

		return NULL;
	}

	memcpy(config->name, name, name_size);

	memcpy(config->path, path, path_size);

	config->parent = parent;

	return config;
}

void configuration_object_instantiate(configuration config, configuration_impl impl)
{
	config->impl = impl;
}

const char * configuration_object_name(configuration config)
{
	return config->name;
}

const char * configuration_object_path(configuration config)
{
	return config->path;
}

configuration configuration_object_parent(configuration config)
{
	return config->parent;
}

const char * configuration_object_source(configuration config)
{
	return config->source;
}

configuration_impl configuration_object_impl(configuration config)
{
	return config->impl;
}

int configuration_object_set(configuration config, const char * key, value v)
{
	return hash_map_insert(config->map, (const hash_map_key)key, v);
}

value configuration_object_get(configuration config, const char * key)
{
	return hash_map_get(config->map, (const hash_map_key)key);
}

int configuration_object_destroy_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args)
{
	(void)map;
	(void)key;
	(void)args;

	if (val != NULL)
	{
		value v = val;

		value_destroy(v);
	}

	return 0;
}

void configuration_object_destroy(configuration config)
{
	if (config->name != NULL)
	{
		free(config->name);
	}

	if (config->path != NULL)
	{
		free(config->path);
	}

	configuration_stream_destroy(config->source);

	hash_map_iterate(config->map, &configuration_object_destroy_cb_iterate, NULL);

	hash_map_destroy(config->map);

	free(config);
}
