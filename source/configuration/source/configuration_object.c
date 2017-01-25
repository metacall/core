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
#include <configuration/configuration_impl.h>

#include <log/log.h>

#include <string.h>

/* -- Forward Declarations -- */

struct configuration_childs_cb_iterator_type;

/* -- Type Definitions -- */

typedef struct configuration_childs_cb_iterator_type * configuration_childs_cb_iterator;

/* -- Member Data -- */

struct configuration_childs_cb_iterator_type
{
	int result;
	configuration parent;
	vector childs;
	set storage;
};

struct configuration_type
{
	char * name;
	char * path;
	set map;
	configuration parent;
	char * source;
	configuration_impl impl;
};

/* -- Private Methods -- */

static int configuration_object_initialize_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

static int configuration_object_childs_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

static int configuration_object_destroy_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

/* -- Methods -- */

int configuration_object_initialize_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	set map = args;

	(void)s;

	if (key != NULL && val != NULL)
	{
		return set_insert(map, key, value_copy(val));
	}

	return 0;
}

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

	config->map = set_create(&hash_callback_str, &comparable_callback_str);

	if (config->name == NULL || config->path == NULL || config->map == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration object initialization");

		configuration_object_destroy(config);

		return NULL;
	}

	memcpy(config->name, name, name_size);

	memcpy(config->path, path, path_size);

	config->parent = parent;

	if (config->parent != NULL)
	{
		set_iterate(config->parent->map, &configuration_object_initialize_cb_iterate, config->map);
	}

	return config;
}

int configuration_object_childs_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	value v = val;

	(void)s;

	if (value_type_id(v) == TYPE_STRING)
	{
		size_t size = value_type_size(v);

		const char * extension = configuration_impl_extension();

		size_t length = strlen(extension);

		if (size > length + 1)
		{
			const char * path = value_to_string(v);

			const char * last = &path[size - length - 1];

			if (strncmp(last, extension, length) == 0)
			{
				configuration_childs_cb_iterator iterator = args;

				if (set_get(iterator->storage, key) == NULL)
				{
					configuration child = configuration_object_initialize(key, path, iterator->parent);

					if (child == NULL)
					{
						iterator->result = 1;

						return 1;
					}

					vector_push_back(iterator->childs, &child);
				}
			}
		}
	}

	return 0;
}

int configuration_object_childs(configuration config, vector childs, set storage)
{
	struct configuration_childs_cb_iterator_type iterator;

	iterator.result = 0;
	iterator.parent = config;
	iterator.childs = childs;
	iterator.storage = storage;

	set_iterate(config->map, &configuration_object_childs_cb_iterate, &iterator);

	return iterator.result;
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
	value original = set_get(config->map, (set_key)key);

	if (original != NULL)
	{
		value_destroy(original);
	}

	return set_insert(config->map, (set_key)key, v);
}

value configuration_object_get(configuration config, const char * key)
{
	return set_get(config->map, (set_key)key);
}

int configuration_object_destroy_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	(void)s;
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

	set_iterate(config->map, &configuration_object_destroy_cb_iterate, NULL);

	set_destroy(config->map);

	free(config);
}
