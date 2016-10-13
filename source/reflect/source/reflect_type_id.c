/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <reflect/reflect_type_id.h>

int type_id_primitive(type_id id)
{
	return !(id < TYPE_INVALID);
}

int type_id_runtime(type_id id)
{
	return !(id > TYPE_INVALID);
}

int type_id_invalid(type_id id)
{
	return !(id == TYPE_INVALID);
}
