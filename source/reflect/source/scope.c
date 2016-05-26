/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <adt/hash_map.h>
#include <adt/hash_map_str.h>

#include <reflect/scope.h>

#include <string.h>

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
			sp->map = hash_map_create(&hash_map_cb_hash_str, &hash_map_cb_compare_str);

			return sp;
		}
	}

	return NULL;
}

int scope_define(scope sp, const char * key, scope_object obj)
{
	if (sp != NULL && key != NULL && obj != NULL)
	{
		return hash_map_insert(sp->map, (hash_map_key)key, (hash_map_value)obj);
	}

	return 1;
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
