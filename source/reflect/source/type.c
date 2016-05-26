/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <reflect/type.h>

#include <string.h>

typedef struct type_type
{
	type_id id;
	char * name;
	type_impl impl;
} * type;

type type_create(type_id id, const char * name, type_impl impl)
{
	if (type_id_invalid(id) != 0 && name != NULL)
	{
		type t = malloc(sizeof(struct type_type));

		if (t)
		{
			t->id = id;
			t->name = strdup(name);
			t->impl = impl;

			return t;
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
		free(t->name);

		free(t);
	}
}
