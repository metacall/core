/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

#include <reflect/reflect_constructor.h>
#include <reflect/reflect_value_type.h>

#include <adt/adt_set.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

#define REFLECT_CONSTRUCTOR_INVALID_INDEX ((size_t)~0)

typedef struct constructor_node_type
{
	size_t index;
	char *name;
	type t;

} * constructor_node;

struct constructor_type
{
	enum class_visibility_id visibility;
	set map;
	size_t count;
};

static constructor_node constructor_head(constructor ctor);
static constructor_node constructor_at(constructor ctor, size_t index);
static value constructor_metadata_args_map_name(const char *name);
static value constructor_metadata_args_map(constructor ctor);
static value constructor_metadata_args(constructor ctor);
static value constructor_metadata_visibility(constructor ctor);

constructor_node constructor_head(constructor ctor)
{
	if (ctor != NULL)
	{
		return (constructor_node)((constructor)(ctor + 1));
	}

	return NULL;
}

constructor_node constructor_at(constructor ctor, size_t index)
{
	constructor_node node_list = constructor_head(ctor);

	if (node_list != NULL && index < ctor->count)
	{
		return &node_list[index];
	}

	return NULL;
}

constructor constructor_create(size_t count, enum class_visibility_id visibility)
{
	constructor ctor = malloc(sizeof(struct constructor_type) + sizeof(struct constructor_node_type) * count);

	if (ctor != NULL)
	{
		size_t index;

		ctor->map = set_create(&hash_callback_str, &comparable_callback_str);

		if (ctor->map == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid constructor set allocation");

			free(ctor);

			return NULL;
		}

		ctor->count = count;

		for (index = 0; index < count; ++index)
		{
			constructor_node node = constructor_at(ctor, index);

			if (node != NULL)
			{
				node->index = REFLECT_CONSTRUCTOR_INVALID_INDEX;
				node->name = NULL;
				node->t = NULL;
			}
		}

		ctor->visibility = visibility;

		return ctor;
	}

	return NULL;
}

size_t constructor_count(constructor ctor)
{
	if (ctor != NULL)
	{
		return ctor->count;
	}

	return 0;
}

size_t constructor_get_index(constructor ctor, const char *name)
{
	if (ctor != NULL && name != NULL)
	{
		constructor_node node = set_get(ctor->map, (set_key)name);

		if (node != NULL)
		{
			return node->index;
		}
	}

	return REFLECT_CONSTRUCTOR_INVALID_INDEX;
}

const char *constructor_get_name(constructor ctor, size_t index)
{
	if (ctor != NULL && index < ctor->count)
	{
		constructor_node node = constructor_at(ctor, index);

		return node->name;
	}

	return NULL;
}

type constructor_get_type(constructor ctor, size_t index)
{
	if (ctor != NULL && index < ctor->count)
	{
		constructor_node node = constructor_at(ctor, index);

		return node->t;
	}

	return NULL;
}

void constructor_set(constructor ctor, size_t index, const char *name, type t)
{
	if (ctor != NULL && index < ctor->count && name != NULL)
	{
		constructor_node node = constructor_at(ctor, index);

		size_t name_size = strlen(name) + 1;

		char *name_node = malloc(sizeof(char) * name_size);

		if (name_node == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid constructor name allocation");

			return;
		}

		if (node->name != NULL)
		{
			free(node->name);
		}

		node->name = name_node;

		memcpy(node->name, name, name_size);

		node->t = t;

		node->index = index;

		if (set_insert(ctor->map, (set_key)node->name, (set_value)node) != 0)
		{
			free(node->name);

			node->index = REFLECT_CONSTRUCTOR_INVALID_INDEX;
			node->name = NULL;
			node->t = NULL;

			log_write("metacall", LOG_LEVEL_ERROR, "Invalid constructor set insertion");
		}
	}
}

int constructor_compare(constructor ctor, type_id args[], size_t size)
{
	size_t iterator;

	if (ctor == NULL)
	{
		return 1;
	}

	if (ctor->count != size)
	{
		return 1;
	}

	for (iterator = 0; iterator < size; ++iterator)
	{
		type t = constructor_get_type(ctor, iterator);

		if (args[iterator] != type_index(t))
		{
			return 1;
		}
	}

	return 0;
}

enum class_visibility_id constructor_visibility(constructor ctor)
{
	if (ctor == NULL)
	{
		return VISIBILITY_PUBLIC; /* Public by default */
	}

	return ctor->visibility;
}

value constructor_metadata_args_map_name(const char *name)
{
	static const char name_str[] = "name";

	value *v_array, v = value_create_array(NULL, 2);

	if (v == NULL)
	{
		return NULL;
	}

	v_array = value_to_array(v);

	v_array[0] = value_create_string(name_str, sizeof(name_str) - 1);

	if (v_array[0] == NULL)
	{
		value_type_destroy(v);

		return NULL;
	}

	v_array[1] = value_create_string(name, strlen(name));

	if (v_array[1] == NULL)
	{
		value_type_destroy(v);

		return NULL;
	}

	return v;
}

value constructor_metadata_args_map(constructor ctor)
{
	value args = value_create_array(NULL, ctor->count);

	if (args == NULL)
	{
		return NULL;
	}

	if (ctor->count > 0)
	{
		value *args_array = value_to_array(args);

		size_t index;

		for (index = 0; index < ctor->count; ++index)
		{
			constructor_node node = constructor_at(ctor, index);

			value *args_map_ptr;

			args_array[index] = value_create_map(NULL, 2);

			if (args_array[index] == NULL)
			{
				value_type_destroy(args);

				return NULL;
			}

			args_map_ptr = value_to_map(args_array[index]);

			args_map_ptr[0] = constructor_metadata_args_map_name(node->name);

			if (args_map_ptr[0] == NULL)
			{
				value_type_destroy(args);

				return NULL;
			}

			args_map_ptr[1] = type_metadata(node->t);

			if (args_map_ptr[1] == NULL)
			{
				value_type_destroy(args);

				return NULL;
			}
		}
	}

	return args;
}

value constructor_metadata_args(constructor ctor)
{
	static const char args_str[] = "args";

	value args_map = constructor_metadata_args_map(ctor);

	if (args_map != NULL)
	{
		value args = value_create_array(NULL, 2);

		value *args_array;

		if (args == NULL)
		{
			value_type_destroy(args);

			return NULL;
		}

		args_array = value_to_array(args);

		args_array[0] = value_create_string(args_str, sizeof(args_str) - 1);

		if (args_array[0] == NULL)
		{
			value_type_destroy(args);

			return NULL;
		}

		args_array[1] = args_map;

		return args;
	}

	return NULL;
}

value constructor_metadata_visibility(constructor ctor)
{
	return class_visibility_value_pair(ctor->visibility);
}

value constructor_metadata(constructor ctor)
{
	value vi, args, sig;
	value *sig_map;

	/* Create return array */
	vi = constructor_metadata_visibility(ctor);

	if (vi == NULL)
	{
		return NULL;
	}

	/* Create arguments map */
	args = constructor_metadata_args(ctor);

	if (args == NULL)
	{
		value_type_destroy(vi);

		return NULL;
	}

	/* Create signature map (return visibility + args) */
	sig = value_create_map(NULL, 2);

	if (sig == NULL)
	{
		value_type_destroy(vi);
		value_type_destroy(args);

		return NULL;
	}

	sig_map = value_to_map(sig);

	sig_map[0] = vi;
	sig_map[1] = args;

	return sig;
}

void constructor_destroy(constructor ctor)
{
	if (ctor != NULL)
	{
		size_t index;

		for (index = 0; index < ctor->count; ++index)
		{
			constructor_node node = constructor_at(ctor, index);

			if (node != NULL && node->name != NULL)
			{
				free(node->name);
			}
		}

		set_destroy(ctor->map);

		free(ctor);
	}
}
