/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <reflect/reflect_value_type.h>
#include <reflect/reflect_value_type_id_size.h>

size_t value_type_id_size(type_id id)
{
	static const size_t type_id_size_list[TYPE_SIZE] =
	{
		sizeof(boolean),	/* TYPE_BOOL */
		sizeof(char),		/* TYPE_CHAR */
		sizeof(short),		/* TYPE_SHORT */
		sizeof(int),		/* TYPE_INT */
		sizeof(long),		/* TYPE_LONG */
		sizeof(float),		/* TYPE_FLOAT */
		sizeof(double),		/* TYPE_DOUBLE */
		sizeof(char *),		/* TYPE_STRING */
		sizeof(void *),		/* TYPE_BUFFER */
		sizeof(value *),	/* TYPE_ARRAY */
		sizeof(set),		/* TYPE_MAP */
		sizeof(void *)		/* TYPE_PTR */
	};

	if (type_id_invalid(id) == 0)
	{
		return 0;
	}

	return type_id_size_list[id];
}
