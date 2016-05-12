/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <reflect/type_impl.h>
#include <reflect/type.h>

type type_create(int id, char * name, size_t size, type_interface interface)
{
	if (id > 0 && name != NULL)
	{
		type t = malloc(sizeof(struct type_type));

		if (t)
		{
			t->id = id;
			t->name = name;
			t->size = size;
			t->interface = interface;

			if (t->interface != NULL && t->interface->create != NULL)
			{
				t->impl = t->interface->create(t);
			}
			else
			{
				t->impl = NULL;
			}

			return t;
		}
	}

	return NULL;
}

int type_id(type t)
{
	if (t != NULL)
	{
		return t->id;
	}

	return TYPE_IMPL_ID_INVALID;
}

char * type_name(type t)
{
	if (t != NULL)
	{
		return t->name;
	}

	return NULL;
}

size_t type_size(type t)
{
	if (t != NULL)
	{
		return t->size;
	}

	return 0;
}

void type_destroy(type t)
{
	if (t != NULL && type_impl_id_runtime(t->id) == 0)
	{
		if (t->interface != NULL && t->interface->destroy != NULL)
		{
			t->interface->destroy(t, t->impl);
		}

		free(t);
	}
}
