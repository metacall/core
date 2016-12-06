/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
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

type_id value_type_id(value v)
{
	size_t size = value_size(v);

	size_t offset = size - sizeof(type_id);

	type_id id = TYPE_INVALID;

	value_to((value)(((uintptr_t)v) + offset), &id, sizeof(type_id));

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
	if (str == NULL || length == 0)
	{
		return NULL;
	}

	return value_type_create(str, length + 1, TYPE_STRING);
}

value value_create_ptr(const void * ptr)
{
	return value_type_create(&ptr, sizeof(const void *), TYPE_PTR);
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

void * value_to_ptr(value v)
{
	uintptr_t * uint_ptr = value_data(v);

	return (void *)(*uint_ptr);
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

value value_from_ptr(value v, const void * ptr)
{
	return value_from(v, &ptr, sizeof(const void *));
}
