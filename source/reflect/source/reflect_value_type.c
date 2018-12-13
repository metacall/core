/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <stdint.h>

/* -- Methods -- */

value value_type_create(const void * data, size_t bytes, type_id id)
{
	value v = value_alloc(bytes + sizeof(type_id));

	if (v == NULL)
	{
		return NULL;
	}

	value_from(v, data, bytes);

	value_from((value)(((uintptr_t)v) + bytes), &id, sizeof(type_id));

	return v;
}

value value_type_copy(value v)
{
	if (v != NULL)
	{
		type_id id = value_type_id(v);

		if (type_id_array(id) == 0)
		{
			size_t index, size = value_type_size(v) / sizeof(value);

			value new_v = value_create_array(NULL, size);

			value * new_v_array = value_to_array(new_v);

			value * v_array = value_to_array(v);

			for (index = 0; index < size; ++index)
			{
				new_v_array[index] = value_type_copy(v_array[index]);
			}
		}
		else if (type_id_map(id) == 0)
		{
			size_t index, size = value_type_size(v) / sizeof(value);

			value new_v = value_create_map(NULL, size);

			value * new_v_map = value_to_map(new_v);

			value * v_map = value_to_map(v);

			for (index = 0; index < size; ++index)
			{
				new_v_map[index] = value_type_copy(v_map[index]);
			}
		}

		if (type_id_invalid(id) != 0)
		{
			return value_copy(v);
		}
	}

	return NULL;
}

size_t value_type_size(value v)
{
	size_t size = value_size(v);

	return size - sizeof(type_id);
}

type_id value_type_id(value v)
{
	type_id id = TYPE_INVALID;

	if (v != NULL)
	{
		size_t size = value_size(v);

		size_t offset = size - sizeof(type_id);

		value_to((value)(((uintptr_t)v) + offset), &id, sizeof(type_id));
	}

	return id;
}

value value_create_bool(boolean b)
{
	return value_type_create(&b, sizeof(boolean), TYPE_BOOL);
}

value value_create_char(char c)
{
	return value_type_create(&c, sizeof(char), TYPE_CHAR);
}

value value_create_short(short s)
{
	return value_type_create(&s, sizeof(short), TYPE_SHORT);
}

value value_create_int(int i)
{
	return value_type_create(&i, sizeof(int), TYPE_INT);
}

value value_create_long(long l)
{
	return value_type_create(&l, sizeof(long), TYPE_LONG);
}

value value_create_float(float f)
{
	return value_type_create(&f, sizeof(float), TYPE_FLOAT);
}

value value_create_double(double d)
{
	return value_type_create(&d, sizeof(double), TYPE_DOUBLE);
}

value value_create_string(const char * str, size_t length)
{
	return value_type_create(str, length + 1, TYPE_STRING);
}

value value_create_buffer(const void * buffer, size_t size)
{
	if (buffer == NULL || size == 0)
	{
		return NULL;
	}

	return value_type_create(buffer, sizeof(char) * size, TYPE_BUFFER);
}

value value_create_array(const value * values, size_t size)
{
	return value_type_create(values, sizeof(const value) * size, TYPE_ARRAY);
}

value value_create_map(const value * tuples, size_t size)
{
	return value_type_create(tuples, sizeof(const value) * size, TYPE_MAP);
}

value value_create_ptr(const void * ptr)
{
	return value_type_create(&ptr, sizeof(const void *), TYPE_PTR);
}

value value_create_null()
{
	return value_type_create(NULL, 0, TYPE_NULL);
}

boolean value_to_bool(value v)
{
	boolean b = 0;

	value_to(v, &b, sizeof(boolean));

	return b;
}

char value_to_char(value v)
{
	char c = '\0';

	value_to(v, &c, sizeof(char));

	return c;
}

short value_to_short(value v)
{
	short s = 0;

	value_to(v, &s, sizeof(short));

	return s;
}

int value_to_int(value v)
{
	int i = 0;

	value_to(v, &i, sizeof(int));

	return i;
}

long value_to_long(value v)
{
	long l = 0L;

	value_to(v, &l, sizeof(long));

	return l;
}

float value_to_float(value v)
{
	float f = 0.0f;

	value_to(v, &f, sizeof(float));

	return f;
}

double value_to_double(value v)
{
	double d = 0.0;

	value_to(v, &d, sizeof(double));

	return d;
}

char * value_to_string(value v)
{
	return value_data(v);
}

void * value_to_buffer(value v)
{
	return value_data(v);
}

value * value_to_array(value v)
{
	return value_data(v);
}

value * value_to_map(value v)
{
	return value_data(v);
}

void * value_to_ptr(value v)
{
	uintptr_t * uint_ptr = value_data(v);

	return (void *)(*uint_ptr);
}

void * value_to_null(value v)
{
	(void)v;

	return (void *)NULL;
}

value value_from_bool(value v, boolean b)
{
	return value_from(v, &b, sizeof(boolean));
}

value value_from_char(value v, char c)
{
	return value_from(v, &c, sizeof(char));
}

value value_from_short(value v, short s)
{
	return value_from(v, &s, sizeof(short));
}

value value_from_int(value v, int i)
{
	return value_from(v, &i, sizeof(int));
}

value value_from_long(value v, long l)
{
	return value_from(v, &l, sizeof(long));
}

value value_from_float(value v, float f)
{
	return value_from(v, &f, sizeof(float));
}

value value_from_double(value v, double d)
{
	return value_from(v, &d, sizeof(double));
}

value value_from_string(value v, const char * str, size_t length)
{
	if (v != NULL && str != NULL && length > 0)
	{
		size_t current_size = value_size(v);

		size_t bytes = length + 1;

		size_t size = (bytes <= current_size) ? bytes : current_size;

		return value_from(v, str, size);
	}

	return v;
}

value value_from_buffer(value v, const void * buffer, size_t size)
{
	if (v != NULL && buffer != NULL && size > 0)
	{
		size_t current_size = value_size(v);

		size_t bytes = sizeof(char) * size;

		return value_from(v, buffer, (bytes <= current_size) ? bytes : current_size);
	}

	return v;
}

value value_from_array(value v, const value * values, size_t size)
{
	if (v != NULL && values != NULL && size > 0)
	{
		size_t current_size = value_size(v);

		size_t bytes = sizeof(const value) * size;

		return value_from(v, values, (bytes <= current_size) ? bytes : current_size);
	}

	return v;
}

value value_from_map(value v, const value * tuples, size_t size)
{
	if (v != NULL && tuples != NULL && size > 0)
	{
		size_t current_size = value_size(v);

		size_t bytes = sizeof(const value) * size;

		return value_from(v, tuples, (bytes <= current_size) ? bytes : current_size);
	}

	return v;
}

value value_from_ptr(value v, const void * ptr)
{
	return value_from(v, &ptr, sizeof(const void *));
}

value value_from_null(value v)
{
	return value_from(v, NULL, 0);
}

void value_type_destroy(value v)
{
	if (v != NULL)
	{
		type_id id = value_type_id(v);

		if (type_id_array(id) == 0)
		{
			size_t index, size = value_type_size(v) / sizeof(value);

			value * v_array = value_to_array(v);

			for (index = 0; index < size; ++index)
			{
				value_type_destroy(v_array[index]);
			}
		}
		else if (type_id_map(id) == 0)
		{
			size_t index, size = value_type_size(v) / sizeof(value);

			value * v_map = value_to_map(v);

			for (index = 0; index < size; ++index)
			{
				value_type_destroy(v_map[index]);
			}
		}

		if (type_id_invalid(id) != 0)
		{
			value_destroy(v);
		}
	}
}
