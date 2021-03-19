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

#include <reflect/reflect_type.h>
#include <reflect/reflect_value_type.h>

#include <stdlib.h>
#include <string.h>

struct type_type
{
	type_id id;
	char *name;
	type_impl impl;
	type_interface interface;
};

static value type_metadata_impl(type t);

type type_create(type_id id, const char *name, type_impl impl, type_impl_interface_singleton singleton)
{
	if (type_id_invalid(id) != 0 && name != NULL)
	{
		type t = malloc(sizeof(struct type_type));

		if (t)
		{
			size_t name_size = strlen(name) + 1;

			t->name = malloc(sizeof(char) * name_size);

			if (t->name == NULL)
			{
				/* error */

				free(t);

				return NULL;
			}

			memcpy(t->name, name, name_size);

			t->id = id;

			t->impl = impl;

			if (singleton != NULL)
			{
				t->interface = singleton();
			}
			else
			{
				t->interface = NULL;
			}

			if (t->interface != NULL && t->interface->create != NULL)
			{
				if (t->interface->create(t, impl) == 0)
				{
					return t;
				}
			}
			else
			{
				return t;
			}

			free(t);
		}
	}

	return NULL;
}

type_id type_index(type t)
{
	if (t != NULL)
	{
		return t->id;
	}

	return TYPE_INVALID;
}

const char *type_name(type t)
{
	if (t != NULL)
	{
		return t->name;
	}

	return NULL;
}

type_impl type_derived(type t)
{
	if (t != NULL)
	{
		return t->impl;
	}

	return NULL;
}

value type_metadata_impl(type t)
{
	static const char name_str[] = "name";
	static const char id_str[] = "id";

	value v, name, id;
	value *v_map, *name_array, *id_array;

	const char *type_name_str = t != NULL ? t->name : "";

	name = value_create_array(NULL, 2);

	if (name == NULL)
	{
		return NULL;
	}

	name_array = value_to_array(name);

	name_array[0] = value_create_string(name_str, sizeof(name_str) - 1);

	if (name_array[0] == NULL)
	{
		value_type_destroy(name);

		return NULL;
	}

	name_array[1] = value_create_string(type_name_str, strlen(type_name_str));

	if (name_array[1] == NULL)
	{
		value_type_destroy(name);

		return NULL;
	}

	id = value_create_array(NULL, 2);

	if (id == NULL)
	{
		value_type_destroy(name);

		return NULL;
	}

	id_array = value_to_array(id);

	id_array[0] = value_create_string(id_str, sizeof(id_str) - 1);

	if (id_array[0] == NULL)
	{
		value_type_destroy(name);

		value_type_destroy(id);

		return NULL;
	}

	id_array[1] = value_create_int(t != NULL ? (int)t->id : (int)TYPE_INVALID);

	if (id_array[1] == NULL)
	{
		value_type_destroy(name);

		value_type_destroy(id);

		return NULL;
	}

	/* Create type map (type name + type id) */
	v = value_create_map(NULL, 2);

	if (v == NULL)
	{
		value_type_destroy(name);

		value_type_destroy(id);

		return NULL;
	}

	v_map = value_to_map(v);

	v_map[0] = name;

	v_map[1] = id;

	return v;
}

value type_metadata(type t)
{
	static const char type_str[] = "type";

	value *v_array, v = value_create_array(NULL, 2);

	if (v == NULL)
	{
		return NULL;
	}

	v_array = value_to_array(v);

	v_array[0] = value_create_string(type_str, sizeof(type_str) - 1);

	if (v_array[0] == NULL)
	{
		value_type_destroy(v);

		return NULL;
	}

	v_array[1] = type_metadata_impl(t);

	if (v_array[1] == NULL)
	{
		value_type_destroy(v);

		return NULL;
	}

	return v;
}

void type_destroy(type t)
{
	if (t != NULL)
	{
		if (t->interface != NULL && t->interface->destroy != NULL)
		{
			t->interface->destroy(t, t->impl);
		}

		free(t->name);

		free(t);
	}
}
