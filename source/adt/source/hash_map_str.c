/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

#include <adt/hash_map.h>
#include <adt/hash_map_str.h>

#include <string.h>

hash_map_hash hash_map_cb_hash_str(hash_map_key key)
{
	char * str = (char *)key;

	hash_map_hash hash = 0x1505;

	while (*str++ != '\0')
	{
		hash = (hash_map_hash)(((hash << 5) + hash) + *str);
	}

	return hash;
}

int hash_map_cb_compare_str(hash_map_key key_a, hash_map_key key_b)
{
	return strcmp((char *)key_a, (char *)key_b);
}
