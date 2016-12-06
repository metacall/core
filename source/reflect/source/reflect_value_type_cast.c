/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
*
*/

/* -- Headers -- */

#include <reflect/reflect_value_type.h>
#include <reflect/reflect_value_type_cast.h>
#include <reflect/reflect_value_type_id_size.h>

#include <stdint.h>

/* -- Methods -- */

value value_type_cast(value v, type_id id)
{
	type_id src_id = value_type_id(v);

	if (type_id_invalid(src_id) == 0 || type_id_invalid(id) == 0)
	{
		return NULL;
	}

	if (value_type_id(v) == id)
	{
		return v;
	}

	/* TODO */

	return NULL;
}
