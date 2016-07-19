/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <adt/hash_map.h>

#include <reflect/scope.h>

#include <string.h>

#include <stdio.h>

typedef struct scope_type
{
	char * name;
	hash_map map;

} * scope;

scope scope_create(const char * name)
{
	if (name != NULL)
	{
		scope sp = malloc(sizeof(struct scope_type));

		if (sp != NULL)
		{
			sp->name = strdup(name);
			sp->map = hash_map_create(&hash_callback_str, &comparable_callback_str);

			return sp;
		}
	}

	return NULL;
}

int scope_object_size(scope sp)
{
	if (sp != NULL)
	{
		return hash_map_element_size(sp->map);
	}

	return -1;
}

int scope_define(scope sp, const char * key, scope_object obj)
{
	if (sp != NULL && key != NULL && obj != NULL)
	{
		return hash_map_insert(sp->map, (hash_map_key)key, (hash_map_value)obj);
	}

	return 1;
}

int scope_print_cb_iterate(hash_map map, hash_map_key key, hash_map_value value, hash_map_cb_iterate_args args)
{
	if (map != NULL && key != NULL && value != NULL && args == NULL)
	{
		printf("Key [%s] -> Value [%p]\n", (char *)key, value);

		return 0;
	}

	return 1;
}

void scope_print(scope sp)
{
	printf("Scope [%s]:\n", sp->name);

	hash_map_iterate(sp->map, &scope_print_cb_iterate, NULL);
}

scope_object scope_get(scope sp, const char * key)
{
	if (sp != NULL && key != NULL)
	{
		return (scope_object)hash_map_get(sp->map, (hash_map_key)key);
	}

	return NULL;
}

scope_object scope_undef(scope sp, const char * key)
{
	if (sp != NULL && key != NULL)
	{
		return (scope_object)hash_map_remove(sp->map, (hash_map_key)key);
	}

	return NULL;
}

int scope_append(scope dest, scope src)
{
	return hash_map_append(dest->map, src->map);
}

void scope_destroy(scope sp)
{
	if (sp)
	{
		hash_map_destroy(sp->map);

		free(sp->name);

		free(sp);
	}
}
