/*
*	Configuration Library by Parra Studios
*	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple configuration formats.
*
*/

/* -- Headers -- */

#include <configuration/configuration_impl.h>
#include <configuration/configuration_object.h>

#include <log/log.h>

#include <portability/portability_path.h>

#include <string.h>

/* -- Member Data -- */

struct configuration_type
{
	char *name;
	char *path;
	set map;
	configuration parent;
	char *source;
	value v;
};

/* -- Private Methods -- */

static char *configuration_object_read(const char *path);

/* -- Methods -- */

char *configuration_object_read(const char *path)
{
	FILE *file = fopen(path, "rb");

	size_t size, size_read;

	char *buffer;

	if (file == NULL)
	{
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

configuration configuration_object_initialize(const char *name, const char *path, configuration parent)
{
	configuration config = malloc(sizeof(struct configuration_type));

	size_t name_size, path_size = 0;

	if (config == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration object allocation");

		return NULL;
	}

	if (path != NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "Trying to load configuration from %s", path);

		config->source = configuration_object_read(path);

		if (config->source == NULL)
		{
			free(config);

			return NULL;
		}
	}
	else
	{
		config->source = NULL;
	}

	name_size = strlen(name) + 1;

	config->name = malloc(name_size * sizeof(char));

	config->map = set_create(&hash_callback_str, &comparable_callback_str);

	if (path != NULL)
	{
		path_size = strlen(path) + 1;

		config->path = malloc(path_size * sizeof(char));

		if (config->path == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration object initialization");

			configuration_object_destroy(config);

			return NULL;
		}
	}
	else
	{
		config->path = NULL;
	}

	if (config->name == NULL || config->map == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration object initialization");

		configuration_object_destroy(config);

		return NULL;
	}

	memcpy(config->name, name, name_size);

	if (config->path && path_size > 0)
	{
		memcpy(config->path, path, path_size);
	}

	config->parent = parent;

	if (config->parent != NULL)
	{
		struct set_iterator_type it;

		for (set_iterator_begin(&it, config->parent->map); set_iterator_end(&it) != 0; set_iterator_next(&it))
		{
			set_insert(config->map, set_iterator_key(&it), set_iterator_value(&it));
		}
	}

	config->v = NULL;

	return config;
}

int configuration_object_childs_valid(set_key key, set_value val)
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

		const char *config_extension = configuration_impl_extension();

		size_t config_length = strlen(config_extension);

		if (config_size > config_length + 1)
		{
			const char *path = value_to_string(v);

			const char *extension_last = &path[config_size - config_length - 1];

			if (strncmp(extension_last, config_extension, config_length) == 0)
			{
				return 0;
			}
		}
	}

	return 1;
}

size_t configuration_object_child_path(configuration config, value v, char *path)
{
	const char *value_path = value_to_string(v);
	size_t size = value_type_size(v);

	if (portability_path_is_absolute(value_path, size) == 0)
	{
		memcpy(path, value_path, size);
	}
	else
	{
		char absolute_path[PORTABILITY_PATH_SIZE];

		size_t absolute_path_size = portability_path_get_directory(config->path, strnlen(config->path, PORTABILITY_PATH_SIZE), absolute_path, PORTABILITY_PATH_SIZE);

		char join_path[PORTABILITY_PATH_SIZE];

		size_t join_path_size = portability_path_join(absolute_path, absolute_path_size, value_path, size, join_path, PORTABILITY_PATH_SIZE);

		size = portability_path_canonical(join_path, join_path_size, path, PORTABILITY_PATH_SIZE);
	}

	return size;
}

int configuration_object_childs(configuration config, vector childs, set storage)
{
	struct set_iterator_type it;

	for (set_iterator_begin(&it, config->map); set_iterator_end(&it) != 0; set_iterator_next(&it))
	{
		set_key key = set_iterator_key(&it);
		set_value val = set_iterator_value(&it);

		if (configuration_object_childs_valid(key, val) == 0)
		{
			if (set_get(storage, key) == NULL)
			{
				char path[PORTABILITY_PATH_SIZE];

				configuration child;

				configuration_object_child_path(config, val, path);

				child = configuration_object_initialize(key, path, config);

				if (child == NULL)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Failed to load configuration %s from %s", (char *)key, path);

					return 1;
				}

				vector_push_back(childs, &child);
			}
		}
	}

	return 0;
}

void configuration_object_instantiate(configuration config, value v)
{
	size_t index, size = value_type_count(v);

	value *v_map = value_to_map(v);

	for (index = 0; index < size; ++index)
	{
		value iterator = v_map[index];

		value *tupla = value_to_array(iterator);

		const char *key = value_to_string(tupla[0]);

		configuration_object_set(config, key, tupla[1]);
	}

	config->v = v;
}

const char *configuration_object_name(configuration config)
{
	return config->name;
}

const char *configuration_object_path(configuration config)
{
	return config->path;
}

configuration configuration_object_parent(configuration config)
{
	return config->parent;
}

const char *configuration_object_source(configuration config)
{
	return config->source;
}

value configuration_object_value(configuration config)
{
	return config->v;
}

int configuration_object_set(configuration config, const char *key, value v)
{
	return set_insert(config->map, (set_key)key, v);
}

value configuration_object_get(configuration config, const char *key)
{
	return set_get(config->map, (set_key)key);
}

int configuration_object_remove(configuration config, const char *key)
{
	return set_remove(config->map, (set_key)key) == NULL;
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

	if (config->source != NULL)
	{
		free(config->source);
	}

	set_destroy(config->map);

	if (config->v != NULL)
	{
		value_type_destroy(config->v);
	}

	free(config);
}
