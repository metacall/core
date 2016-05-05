/************************************************************************/
/*	Reflect Library by Parra Studios									*/
/*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																		*/
/*	A library for provide reflection and metadata representation.		*/
/*																		*/
/************************************************************************/

#include <adt/hash_map.h>
#include <adt/hash_map_str.h>

#include <reflect/scope.h>

typedef struct scope_type
{
	char * name;
	hash_map map;

} * scope;

scope scope_create(char * name)
{
	if (name != NULL)
	{
		scope s = malloc(sizeof(struct scope_type));

		if (s != NULL)
		{
			s->name = name;
			s->map = hash_map_create(&hash_map_cb_hash_str, &hash_map_cb_compare_str);

			return s;
		}
	}

	return NULL;
}

int scope_define(scope s, char * key, scope_object obj)
{
	if (s != NULL && key != NULL && obj != NULL)
	{
		return hash_map_insert(s->map, key, obj);
	}

	return 1;
}

scope_object scope_get(scope s, char * key)
{
	if (s != NULL && key != NULL)
	{
		return (scope_object)hash_map_get(s->map, key);
	}

	return NULL;
}

scope_object scope_undef(scope s, char * key)
{
	if (s != NULL && key != NULL)
	{
		return (scope_object)hash_map_remove(s->map, key);
	}

	return NULL;
}

void scope_destroy(scope s)
{
	if (s)
	{
		hash_map_destroy(s->map);

		free(s);
	}
}
