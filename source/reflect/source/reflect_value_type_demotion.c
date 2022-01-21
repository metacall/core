/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

/* -- Headers -- */

#include <reflect/reflect_value_type.h>
#include <reflect/reflect_value_type_demotion.h>
#include <reflect/reflect_value_type_id_size.h>

/* -- Methods -- */

value value_type_demotion_boolean(value v, type_id id)
{
	union
	{
		char b[8];
		float f;
		double d;
	} buffer = {
		{ 0 }
	};

	value demotion = NULL;

	type_id v_id = value_type_id(v);

	if (!(type_id_integer(v_id) == 0 || type_id_decimal(v_id) == 0) || type_id_boolean(id) != 0)
	{
		return NULL;
	}

	if (v_id == id)
	{
		return v;
	}

	value_to(v, (void *)&buffer.b[0], value_type_id_size(v_id));

	demotion = value_type_create(NULL, value_type_id_size(id), id);

	if (demotion == NULL)
	{
		return v;
	}

	value_destroy(v);

	if (type_id_decimal(v_id) == 0)
	{
		if (v_id == TYPE_FLOAT)
		{
			if (buffer.f != 0.0f && buffer.f != -0.0f)
			{
				return value_from_bool(demotion, 1L);
			}
		}
		else if (v_id == TYPE_DOUBLE)
		{
			if (buffer.d != 0.0 && buffer.d != -0.0)
			{
				return value_from_bool(demotion, 1L);
			}
		}
		else
		{
			/* error */
		}
	}
	else
	{
		size_t iterator;

		for (iterator = 0; iterator < 8; ++iterator)
		{
			if (buffer.b[iterator] != 0x00)
			{
				return value_from_bool(demotion, 1L);
			}
		}
	}

	return demotion;
}

value value_type_demotion_integer(value v, type_id id)
{
	char buffer[8] = { 0 };

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

	value_to(v, (void *)&buffer[0], value_type_id_size(v_id));

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
