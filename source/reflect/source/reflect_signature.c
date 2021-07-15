/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <reflect/reflect_signature.h>
#include <reflect/reflect_value_type.h>

#include <adt/adt_set.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

#define REFLECT_SIGNATURE_INVALID_INDEX ((size_t)~0)

typedef struct signature_node_type
{
	size_t index;
	char *name;
	type t;

} * signature_node;

struct signature_type
{
	type ret;
	set map;
	size_t count;
};

static signature_node signature_head(signature s);

static signature_node signature_at(signature s, size_t index);

static value signature_metadata_return(signature s);

static value signature_metadata_args_map_name(const char *name);

static value signature_metadata_args_map(signature s);

static value signature_metadata_args(signature s);

signature_node signature_head(signature s)
{
	if (s != NULL)
	{
		return (signature_node)((signature)(s + 1));
	}

	return NULL;
}

signature_node signature_at(signature s, size_t index)
{
	signature_node node_list = signature_head(s);

	if (node_list != NULL && index < s->count)
	{
		return &node_list[index];
	}

	return NULL;
}

signature signature_create(size_t count)
{
	signature s = malloc(sizeof(struct signature_type) + sizeof(struct signature_node_type) * count);

	if (s != NULL)
	{
		size_t index;

		s->map = set_create(&hash_callback_str, &comparable_callback_str);

		if (s->map == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid signature set allocation");

			free(s);

			return NULL;
		}

		s->ret = NULL;

		s->count = count;

		for (index = 0; index < count; ++index)
		{
			signature_node node = signature_at(s, index);

			if (node != NULL)
			{
				node->index = REFLECT_SIGNATURE_INVALID_INDEX;
				node->name = NULL;
				node->t = NULL;
			}
		}

		return s;
	}

	return NULL;
}

signature signature_resize(signature s, size_t count)
{
	signature new_s;
	size_t index;

	if (s == NULL)
	{
		return NULL;
	}

	new_s = realloc(s, sizeof(struct signature_type) + sizeof(struct signature_node_type) * count);

	if (new_s == NULL)
	{
		return NULL;
	}

	for (index = new_s->count; index < count; ++index)
	{
		signature_node node = signature_at(new_s, index);

		if (node != NULL)
		{
			node->index = REFLECT_SIGNATURE_INVALID_INDEX;
			node->name = NULL;
			node->t = NULL;
		}
	}

	new_s->count = count;

	return new_s;
}

size_t signature_count(signature s)
{
	if (s != NULL)
	{
		return s->count;
	}

	return 0;
}

size_t signature_get_index(signature s, const char *name)
{
	if (s != NULL && name != NULL)
	{
		signature_node node = set_get(s->map, (set_key)name);

		if (node != NULL)
		{
			return node->index;
		}
	}

	return REFLECT_SIGNATURE_INVALID_INDEX;
}

const char *signature_get_name(signature s, size_t index)
{
	if (s != NULL && index < s->count)
	{
		signature_node node = signature_at(s, index);

		return node->name;
	}

	return NULL;
}

type signature_get_type(signature s, size_t index)
{
	if (s != NULL && index < s->count)
	{
		signature_node node = signature_at(s, index);

		return node->t;
	}

	return NULL;
}

type signature_get_return(signature s)
{
	if (s != NULL)
	{
		return s->ret;
	}

	return NULL;
}

void signature_set(signature s, size_t index, const char *name, type t)
{
	if (s != NULL && index < s->count && name != NULL)
	{
		signature_node node = signature_at(s, index);

		size_t name_size = strlen(name) + 1;

		char *name_node = malloc(sizeof(char) * name_size);

		if (name_node == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid signature name allocation");

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

		if (set_insert(s->map, (set_key)node->name, (set_value)node) != 0)
		{
			free(node->name);

			node->index = REFLECT_SIGNATURE_INVALID_INDEX;
			node->name = NULL;
			node->t = NULL;

			log_write("metacall", LOG_LEVEL_ERROR, "Invalid signature set insertion");
		}
	}
}

void signature_set_return(signature s, type t)
{
	if (s != NULL)
	{
		s->ret = t;
	}
}

int signature_compare(signature s, type_id ret, type_id args[], size_t size)
{
	size_t iterator;

	if (s == NULL)
	{
		return 1;
	}

	if (s->count != size)
	{
		return 1;
	}

	if (type_index(s->ret) != ret)
	{
		return 1;
	}

	for (iterator = 0; iterator < size; ++iterator)
	{
		type t = signature_get_type(s, iterator);

		if (args[iterator] != type_index(t))
		{
			return 1;
		}
	}

	return 0;
}

value signature_metadata_return(signature s)
{
	static const char ret_str[] = "ret";

	value ret = value_create_array(NULL, 2);

	value *ret_array, *ret_map;

	if (ret == NULL)
	{
		return NULL;
	}

	ret_array = value_to_array(ret);

	ret_array[0] = value_create_string(ret_str, sizeof(ret_str) - 1);

	if (ret_array[0] == NULL)
	{
		value_type_destroy(ret);

		return NULL;
	}

	ret_array[1] = value_create_map(NULL, 1);

	if (ret_array[1] == NULL)
	{
		value_type_destroy(ret);

		return NULL;
	}

	ret_map = value_to_map(ret_array[1]);

	ret_map[0] = type_metadata(s->ret);

	if (ret_map[0] == NULL)
	{
		value_type_destroy(ret);

		return NULL;
	}

	return ret;
}

value signature_metadata_args_map_name(const char *name)
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

value signature_metadata_args_map(signature s)
{
	value args = value_create_array(NULL, s->count);

	if (args == NULL)
	{
		return NULL;
	}

	if (s->count > 0)
	{
		value *args_array = value_to_array(args);

		size_t index;

		for (index = 0; index < s->count; ++index)
		{
			signature_node node = signature_at(s, index);

			value *args_map_ptr;

			args_array[index] = value_create_map(NULL, 2);

			if (args_array[index] == NULL)
			{
				value_type_destroy(args);

				return NULL;
			}

			args_map_ptr = value_to_map(args_array[index]);

			args_map_ptr[0] = signature_metadata_args_map_name(node->name);

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

value signature_metadata_args(signature s)
{
	static const char args_str[] = "args";

	value args_map = signature_metadata_args_map(s);

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

value signature_metadata(signature s)
{
	value ret, args, sig;

	value *sig_map;

	/* Create return array */
	ret = signature_metadata_return(s);

	if (ret == NULL)
	{
		return NULL;
	}

	/* Create arguments map */
	args = signature_metadata_args(s);

	if (args == NULL)
	{
		value_type_destroy(ret);

		return NULL;
	}

	/* Create signature map (return type + args) */
	sig = value_create_map(NULL, 2);

	if (sig == NULL)
	{
		value_type_destroy(ret);

		value_type_destroy(args);

		return NULL;
	}

	sig_map = value_to_map(sig);

	sig_map[0] = ret;

	sig_map[1] = args;

	return sig;
}

void signature_destroy(signature s)
{
	if (s != NULL)
	{
		size_t index;

		for (index = 0; index < s->count; ++index)
		{
			signature_node node = signature_at(s, index);

			if (node != NULL && node->name != NULL)
			{
				free(node->name);
			}
		}

		set_destroy(s->map);

		free(s);
	}
}
