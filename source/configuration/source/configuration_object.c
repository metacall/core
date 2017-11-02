/*
*	Configuration Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple configuration formats.
*
*/

/* -- Headers -- */

#include <configuration/configuration_object.h>
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
	value v;
};

/* -- Private Methods -- */

static int configuration_object_initialize_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

static char * configuration_object_read(const char * path);

static int configuration_object_childs_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

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

char * configuration_object_read(const char * path)
{
	FILE * file = fopen(path, "rb");

	size_t size, size_read;

	char * buffer;

	if (file == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration file path (%s)", path);

		return NULL;
	}

	fseek(file, 0, SEEK_END);

	size = ftell(file);

	fseek(file, 0, SEEK_SET);

	buffer = malloc(sizeof(char) * (size + 1));

	if (buffer == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration file stream allocation");

		fclose(file);

		return NULL;
	}

	size_read = fread(buffer, sizeof(char), size, file);

	fclose(file);

	if (size_read != size)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration file size read (%u != %u)", size_read, size);

		free(buffer);

		return NULL;
	}

	buffer[size_read] = '\0';

	return buffer;
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

	config->source = configuration_object_read(path);

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

int configuration_object_childs_cb_iterate_valid(set_key key, set_value val)
{
	value v = val;

	(void)key;

	/*
	static const char config_name[] = "_loader";

	const char * name = key;

	size_t name_length = strlen(name), config_name_size = sizeof(config_name);

	const char * name_last = &name[name_length - config_name_size + 1];

	if (strcmp(name_last, config_name) != 0)
	{
		return 1;
	}
	*/

	if (value_type_id(v) == TYPE_STRING)
	{
		size_t config_size = value_type_size(v);

		const char * config_extension = configuration_impl_extension();

		size_t config_length = strlen(config_extension);

		if (config_size > config_length + 1)
		{
			const char * path = value_to_string(v);

			const char * extension_last = &path[config_size - config_length - 1];

			if (strncmp(extension_last, config_extension, config_length) == 0)
			{
				return 0;
			}
		}
	}

	return 1;
}

int configuration_object_childs_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	(void)s;

	if (configuration_object_childs_cb_iterate_valid(key, val) == 0)
	{
		configuration_childs_cb_iterator iterator = args;

		if (set_get(iterator->storage, key) == NULL)
		{
			value v = val;

			const char * path = value_to_string(v);

			configuration child = configuration_object_initialize(key, path, iterator->parent);

			if (child == NULL)
			{
				iterator->result = 1;

				return 1;
			}

			vector_push_back(iterator->childs, &child);
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

void configuration_object_instantiate(configuration config, value v)
{
	size_t index, size = value_type_size(v) / sizeof(value);

	value * v_map = value_to_map(v);

	for (index = 0; index < size; ++index)
	{
		value iterator = v_map[index];

		value * tupla = value_to_array(iterator);

		const char * key = value_to_string(tupla[0]);

		configuration_object_set(config, key, tupla[1]);
	}

	config->v = v;
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

value configuration_object_value(configuration config)
{
	return config->v;
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

	if (config->v != NULL)
	{
		value_type_destroy(config->v);
	}

	if (config->source != NULL)
	{
		free(config->source);
	}

	set_destroy(config->map);

	free(config);
}
