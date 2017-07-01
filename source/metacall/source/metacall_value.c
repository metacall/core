/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

/* -- Headers -- */

#include <metacall/metacall_value.h>

#include <reflect/reflect_value.h>
#include <reflect/reflect_value_type.h>
#include <reflect/reflect_value_type_stringify.h>

#include <preprocessor/preprocessor_concatenation.h>

#include <assert.h>

#ifndef static_assert

#	define static_assert_impl_expr(predicate, expr) \
	typedef char expr[2 * !!(predicate) - 1]

#	define static_assert_local_impl_expr(predicate, expr) \
	static_assert_impl_expr(predicate, expr); \
	expr PREPROCESSOR_CONCAT(expr, _var); \
	(void) PREPROCESSOR_CONCAT(expr, _var)

#	if defined(__COUNTER__)
#		define static_assert_impl_line(macro, predicate, expr) macro((predicate), PREPROCESSOR_CONCAT(expr, __COUNTER__))
#	elif defined(__LINE__)
		/* WARNING: It can collide if it's used in header files */
#		define static_assert_impl_line(macro, predicate, expr) macro((predicate), PREPROCESSOR_CONCAT(expr, __LINE__))
#	else
#		define static_assert_impl_line(macro, predicate, expr) macro((predicate), expr)
#	endif

#	define static_assert_impl(macro, predicate) static_assert_impl_line(macro, predicate, static_assert_)

#	define static_assert(predicate) static_assert_impl(static_assert_impl_expr, predicate)

#	define static_assert_local(predicate) static_assert_impl(static_assert_local_impl_expr, predicate)

#endif

/* -- Methods -- */

void * metacall_value_create_bool(boolean b)
{
	return value_create_bool(b);
}

void * metacall_value_create_char(char c)
{
	return value_create_char(c);
}

void * metacall_value_create_short(short s)
{
	return value_create_short(s);
}

void * metacall_value_create_int(int i)
{
	return value_create_int(i);
}

void * metacall_value_create_long(long l)
{
	return value_create_long(l);
}

void * metacall_value_create_float(float f)
{
	return value_create_float(f);
}

void * metacall_value_create_double(double d)
{
	return value_create_double(d);
}

void * metacall_value_create_string(const char * str, size_t length)
{
	return value_create_string(str, length);
}

void * metacall_value_create_array(const void * arr, size_t element_size, size_t size)
{
	return value_create_array(arr, element_size, size);
}

void * metacall_value_create_list(const void * values[], size_t size)
{
	return value_create_list((const value *)values, size);
}

void * metacall_value_create_ptr(const void * ptr)
{
	return value_create_ptr(ptr);
}

size_t metacall_value_size(void * v)
{
	return value_type_size(v);
}

enum metacall_value_id metacall_value_id(void * v)
{
	type_id id = value_type_id(v);

	if (id >= 0 && id < TYPE_SIZE)
	{
		static enum metacall_value_id value_id_map[] =
		{
			METACALL_BOOL,
			METACALL_CHAR,
			METACALL_SHORT,
			METACALL_INT,
			METACALL_LONG,
			METACALL_FLOAT,
			METACALL_DOUBLE,
			METACALL_STRING,
			METACALL_ARRAY,
			METACALL_LIST,
			METACALL_PTR,

			METACALL_SIZE,
			METACALL_INVALID
		};

		static_assert_local(((int) TYPE_BOOL == (int) METACALL_BOOL) &&
			((int) TYPE_CHAR == (int) METACALL_CHAR) &&
			((int) TYPE_SHORT == (int) METACALL_SHORT) &&
			((int) TYPE_INT == (int) METACALL_INT) &&
			((int) TYPE_LONG == (int) METACALL_LONG) &&
			((int) TYPE_FLOAT == (int) METACALL_FLOAT) &&
			((int) TYPE_DOUBLE == (int) METACALL_DOUBLE) &&
			((int) TYPE_STRING == (int) METACALL_STRING) &&
			((int) TYPE_ARRAY == (int) METACALL_ARRAY) &&
			((int) TYPE_LIST == (int) METACALL_LIST) &&
			((int) TYPE_PTR == (int) METACALL_PTR) &&
			((int) TYPE_SIZE == (int) METACALL_SIZE) &&
			((int) TYPE_INVALID == (int) METACALL_INVALID));

		return value_id_map[id];
	}

	return METACALL_INVALID;
}

boolean metacall_value_to_bool(void * v)
{
	assert(value_type_id(v) == TYPE_BOOL);

	return value_to_bool(v);
}

char metacall_value_to_char(void * v)
{
	assert(value_type_id(v) == TYPE_CHAR);

	return value_to_char(v);
}

short metacall_value_to_short(void * v)
{
	assert(value_type_id(v) == TYPE_SHORT);

	return value_to_short(v);
}

int metacall_value_to_int(void * v)
{
	assert(value_type_id(v) == TYPE_INT);

	return value_to_int(v);
}

long metacall_value_to_long(void * v)
{
	assert(value_type_id(v) == TYPE_LONG);

	return value_to_long(v);
}

float metacall_value_to_float(void * v)
{
	assert(value_type_id(v) == TYPE_FLOAT);

	return value_to_float(v);
}

double metacall_value_to_double(void * v)
{
	assert(value_type_id(v) == TYPE_DOUBLE);

	return value_to_double(v);
}

char * metacall_value_to_string(void * v)
{
	assert(value_type_id(v) == TYPE_STRING);

	return value_to_string(v);
}

void * metacall_value_to_array(void * v)
{
	assert(value_type_id(v) == TYPE_ARRAY);

	return value_to_array(v);
}

void ** metacall_value_to_list(void * v)
{
	assert(value_type_id(v) == TYPE_LIST);

	return value_to_list(v);
}

void * metacall_value_to_ptr(void * v)
{
	assert(value_type_id(v) == TYPE_PTR);

	return value_to_ptr(v);
}

void * metacall_value_from_bool(void * v, boolean b)
{
	return value_from_bool(v, b);
}

void * metacall_value_from_char(void * v, char c)
{
	return value_from_char(v, c);
}

void * metacall_value_from_short(void * v, short s)
{
	return value_from_short(v, s);
}

void * metacall_value_from_int(void * v, int i)
{
	return value_from_int(v, i);
}

void * metacall_value_from_long(void * v, long l)
{
	return value_from_long(v, l);
}

void * metacall_value_from_float(void * v, float f)
{
	return value_from_float(v, f);
}

void * metacall_value_from_double(void * v, double d)
{
	return value_from_double(v, d);
}

void * metacall_value_from_string(void * v, const char * str, size_t length)
{
	return value_from_string(v, str, length);
}

void * metacall_value_from_array(void * v, const void * arr, size_t element_size, size_t size)
{
	return value_from_array(v, arr, element_size, size);
}

void * metacall_value_from_list(void * v, const void * values[], size_t size)
{
	return value_from_list(v, (const value *)values, size);
}

void * metacall_value_from_ptr(void * v, const void * ptr)
{
	return value_from_ptr(v, ptr);
}

boolean metacall_value_cast_bool(void ** v)
{
	if (value_type_id(*v) != TYPE_BOOL)
	{
		*v = value_type_cast(*v, TYPE_BOOL);
	}

	return value_to_bool(*v);
}

char metacall_value_cast_char(void ** v)
{
	if (value_type_id(*v) != TYPE_CHAR)
	{
		*v = value_type_cast(*v, TYPE_CHAR);
	}

	return value_to_char(*v);
}

short metacall_value_cast_short(void ** v)
{
	if (value_type_id(*v) != TYPE_SHORT)
	{
		*v = value_type_cast(*v, TYPE_SHORT);
	}

	return value_to_short(*v);
}

int metacall_value_cast_int(void ** v)
{
	if (value_type_id(*v) != TYPE_INT)
	{
		*v = value_type_cast(*v, TYPE_INT);
	}

	return value_to_int(*v);
}

long metacall_value_cast_long(void ** v)
{
	if (value_type_id(*v) != TYPE_LONG)
	{
		*v = value_type_cast(*v, TYPE_LONG);
	}

	return value_to_long(*v);
}

float metacall_value_cast_float(void ** v)
{
	if (value_type_id(*v) != TYPE_FLOAT)
	{
		*v = value_type_cast(*v, TYPE_FLOAT);
	}

	return value_to_float(*v);
}

double metacall_value_cast_double(void ** v)
{
	if (value_type_id(*v) != TYPE_DOUBLE)
	{
		*v = value_type_cast(*v, TYPE_DOUBLE);
	}

	return value_to_double(*v);
}

char * metacall_value_cast_string(void ** v)
{
	if (value_type_id(*v) != TYPE_STRING)
	{
		*v = value_type_cast(*v, TYPE_STRING);
	}

	return value_to_string(*v);
}

void * metacall_value_cast_array(void ** v)
{
	if (value_type_id(*v) != TYPE_ARRAY)
	{
		*v = value_type_cast(*v, TYPE_ARRAY);
	}

	return value_to_array(*v);
}

void ** metacall_value_cast_list(void ** v)
{
	if (value_type_id(*v) != TYPE_LIST)
	{
		*v = value_type_cast(*v, TYPE_LIST);
	}

	return value_to_list(*v);
}

void * metacall_value_cast_ptr(void ** v)
{
	if (value_type_id(*v) != TYPE_PTR)
	{
		*v = value_type_cast(*v, TYPE_PTR);
	}

	return value_to_ptr(*v);
}

void metacall_value_stringify(void * v, char * dest, size_t size, size_t * length)
{
	value_stringify(v, dest, size, length);
}

void metacall_value_destroy(void * v)
{
	value_destroy(v);
}
