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
#include <reflect/reflect_value_type_stringify.h>
#include <reflect/reflect_value_type_parse.h>

#include <stdint.h>

/* -- Methods -- */

value value_type_cast(value v, type_id id)
{
	type_id src_id = value_type_id(v);

	size_t src_size = value_type_id_size(src_id);

	size_t dest_size = value_type_id_size(id);

	/* Invalid value type */
	if (type_id_invalid(src_id) == 0 || type_id_invalid(id) == 0)
	{
		return NULL;
	}

	/* Equal types, avoid casting */
	if (src_id == id)
	{
		return v;
	}

	/* Cast from string to any type */
	if (type_id_string(src_id) == 0)
	{
		return value_type_parse(v, id);
	}

	/* Cast from any type to string */
	if (type_id_string(id) == 0)
	{
		return value_type_stringify(v);
	}

	/* Convert single value to buffer */
	if (type_id_buffer(id) == 0 && src_id < TYPE_BUFFER)
	{
		value dest = value_type_create(value_data(v), value_type_id_size(src_id), TYPE_BUFFER);

		if (dest == NULL)
		{
			return NULL;
		}

		value_destroy(v);

		return dest;
	}

	/* TODO: Convert buffer to array */

	/* TODO: Convert buffer of size 1 to a single type */

	/* Convert single value to array */
	if (type_id_array(id) == 0 && src_id < TYPE_BUFFER)
	{
		value dest = value_type_create(&v, sizeof(value), TYPE_ARRAY);

		if (dest == NULL)
		{
			return NULL;
		}

		value_destroy(v);

		return dest;
	}

	/* TODO: Convert array to buffer */

	/* Convert array of size 1 to a single type */
	if (type_id_array(src_id) == 0 && id < TYPE_BUFFER && value_type_size(v) == sizeof(value))
	{
		value * values = value_data(v);

		value dest = values[0];

		if (dest == NULL)
		{
			return NULL;
		}

		if (value_type_id(dest) != id && value_type_id(dest) < TYPE_BUFFER)
		{
			value cast = value_type_cast(dest, id);

			if (cast == NULL)
			{
				return NULL;
			}

			dest = cast;
		}

		value_destroy(v);

		return dest;
	}

	/* TODO: Map */

	/* Promote value type */
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

	/* Demote value type */
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
