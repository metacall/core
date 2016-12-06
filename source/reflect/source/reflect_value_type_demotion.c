/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
*
*/

/* -- Headers -- */

#include <reflect/reflect_value_type.h>
#include <reflect/reflect_value_type_demotion.h>
#include <reflect/reflect_value_type_id_size.h>

/* -- Methods -- */

value value_type_demotion_integer(value v, type_id id)
{
	char buffer[sizeof(long)];

	value demotion = NULL;

	type_id v_id = value_type_id(v);

	if (type_id_integer(v_id) != 0 || type_id_integer(id) != 0 || v_id < id)
	{
		return NULL;
	}

	if (v_id == id)
	{
		return v;
	}

	value_to(v, (void *)&buffer[0], value_size(v));

	demotion = value_type_create((void *)&buffer[0], value_type_id_size(id), id);

	if (demotion == NULL)
	{
		return v;
	}

	value_destroy(v);

	return demotion;
}

value value_type_demotion_decimal(value v, type_id id)
{
	struct
	{
		float f;
		double d;
	} decimal_demotion;

	value demotion = NULL;

	type_id v_id = value_type_id(v);

	if (type_id_decimal(v_id) != 0 || type_id_decimal(id) != 0 || v_id < id)
	{
		return NULL;
	}

	if (v_id == id)
	{
		return v;
	}

	decimal_demotion.d = value_to_double(v);

	decimal_demotion.f = (float)decimal_demotion.d;

	demotion = value_create_float(decimal_demotion.f);

	if (demotion == NULL)
	{
		return v;
	}

	value_destroy(v);

	return demotion;
}
