/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
*
*/

/* -- Headers -- */

#include <reflect/reflect_value_type.h>
#include <reflect/reflect_value_type_cast.h>
#include <reflect/reflect_value_type_id_size.h>
#include <reflect/reflect_value_type_promotion.h>
#include <reflect/reflect_value_type_demotion.h>

#include <stdint.h>

/* -- Methods -- */

value value_type_cast(value v, type_id id)
{
	type_id src_id = value_type_id(v);

	size_t src_size = value_type_id_size(src_id);

	size_t dest_size = value_type_id_size(id);

	if (type_id_invalid(src_id) == 0 || type_id_invalid(id) == 0)
	{
		return NULL;
	}

	if (src_id == id)
	{
		return v;
	}

	if (src_id < id)
	{
		if (type_id_integer(src_id) == 0 && type_id_integer(id) == 0)
		{
			return value_type_promotion_integer(v, id);
		}

		if (type_id_decimal(src_id) == 0 && type_id_decimal(id) == 0)
		{
			return value_type_promotion_decimal(v, id);
		}

		if (type_id_integer(src_id) == 0 && type_id_decimal(id) == 0)
		{
			value dest = NULL;

			int64_t data = 0L;

			value_to(v, &data, src_size);

			if (src_size == dest_size)
			{
				value_from((value)(((uintptr_t)v) + src_size), &id, sizeof(type_id));

				dest = v;
			}
			else
			{
				dest = value_type_create(NULL, dest_size, id);

				if (dest != NULL)
				{
					value_destroy(v);
				}
			}

			if (id == TYPE_FLOAT)
			{
				float f = (float)data;

				return value_from_float(dest, f);
			}
			else if (id == TYPE_DOUBLE)
			{
				double d = (double)data;

				return value_from_double(dest, d);
			}

			return NULL;
		}

		return NULL;
	}

	if (src_id > id)
	{
		if (type_id_boolean(id) == 0)
		{
			return value_type_demotion_boolean(v, id);
		}

		if (type_id_integer(src_id) == 0 && type_id_integer(id) == 0)
		{
			return value_type_demotion_integer(v, id);
		}

		if (type_id_decimal(src_id) == 0 && type_id_decimal(id) == 0)
		{
			return value_type_demotion_decimal(v, id);
		}

		if (type_id_decimal(src_id) == 0 && type_id_integer(id) == 0)
		{
			value dest = NULL;

			int64_t data = 0L;

			if (src_id == TYPE_FLOAT)
			{
				data = (int64_t)value_to_float(v);
			}
			else if (src_id == TYPE_DOUBLE)
			{
				data = (int64_t)value_to_double(v);
			}
			else
			{
				return NULL;
			}

			if (src_size == dest_size)
			{
				value_from((value)(((uintptr_t)v) + src_size), &id, sizeof(type_id));

				dest = v;
			}
			else
			{
				dest = value_type_create(NULL, dest_size, id);

				if (dest != NULL)
				{
					value_destroy(v);
				}
			}

			return value_from(dest, &data, dest_size);
		}

		return NULL;
	}

	return NULL;
}
