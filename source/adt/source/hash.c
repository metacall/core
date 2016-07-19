/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

#include <adt/hash.h>

#include <string.h>

hash hash_callback_str(const hash_key key)
{
	const char * str = (const char *)key;

	hash h = 0x1505;

	while (*str++ != '\0')
	{
		h = (hash)(((h << 5) + h) + *str);
	}

	return h;
}
