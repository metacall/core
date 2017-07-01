/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <reflect/reflect_type_id.h>

int type_id_boolean(type_id id)
{
	return !(id == TYPE_BOOL);
}

int type_id_char(type_id id)
{
	return !(id == TYPE_CHAR);
}

int type_id_integer(type_id id)
{
	return !(id == TYPE_BOOL || id == TYPE_CHAR || id == TYPE_SHORT ||
		id == TYPE_INT || id == TYPE_LONG);
}

int type_id_decimal(type_id id)
{
	return !(id == TYPE_FLOAT || id == TYPE_DOUBLE);
}

int type_id_string(type_id id)
{
	return !(id == TYPE_STRING);
}

int type_id_array(type_id id)
{
	return !(id == TYPE_ARRAY);
}

int type_id_list(type_id id)
{
	return !(id == TYPE_LIST);
}

int type_id_pointer(type_id id)
{
	return !(id == TYPE_PTR);
}

int type_id_invalid(type_id id)
{
	return !(id >= TYPE_SIZE);
}
