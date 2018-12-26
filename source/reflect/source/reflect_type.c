/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <stdlib.h>
#include <string.h>

struct type_type
{
	type_id id;
	char * name;
	type_impl impl;
	type_interface interface;

};

type type_create(type_id id, const char * name, type_impl impl, type_impl_interface_singleton singleton)
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

const char * type_name(type t)
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
