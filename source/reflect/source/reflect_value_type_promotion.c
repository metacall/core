/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
#include <reflect/reflect_value_type_id_size.h>
#include <reflect/reflect_value_type_promotion.h>

/* -- Methods -- */

value value_type_promotion_integer(value v, type_id id)
{
	int64_t data = 0;

	value promotion = NULL;

	type_id v_id = value_type_id(v);

	if (type_id_integer(v_id) != 0 || type_id_integer(id) != 0 || v_id > id)
	{
		return NULL;
	}

	if (v_id == id)
	{
		return v;
	}

	if (v_id == TYPE_BOOL)
	{
		data = (int64_t)value_to_bool(v);
	}
	else if (v_id == TYPE_CHAR)
	{
		data = (int64_t)(signed char)value_to_char(v);
	}
	else if (v_id == TYPE_SHORT)
	{
		data = (int64_t)value_to_short(v);
	}
	else if (v_id == TYPE_INT)
	{
		data = (int64_t)value_to_int(v);
	}
	else
	{
		return NULL;
	}

	promotion = value_type_create((void *)&data, value_type_id_size(id), id);

	if (promotion == NULL)
	{
		return v;
	}

	value_destroy(v);

	return promotion;
}

value value_type_promotion_decimal(value v, type_id id)
{
	struct
	{
		float f;
		double d;
	} decimal_promotion = {
		0.0f, 0.0
	};

	value promotion = NULL;

	type_id v_id = value_type_id(v);

	if (type_id_decimal(v_id) != 0 || type_id_decimal(id) != 0 || v_id > id)
	{
		return NULL;
	}

	if (v_id == id)
	{
		return v;
	}

	decimal_promotion.f = value_to_float(v);

	decimal_promotion.d = (double)decimal_promotion.f;

	promotion = value_create_double(decimal_promotion.d);

	if (promotion == NULL)
	{
		return v;
	}

	value_destroy(v);

	return promotion;
}
