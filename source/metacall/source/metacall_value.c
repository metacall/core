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
			((int) TYPE_PTR == (int) METACALL_PTR) &&
			((int) TYPE_SIZE == (int) METACALL_SIZE) &&
			((int) TYPE_INVALID == (int) METACALL_INVALID));

		return value_id_map[id];
	}

	return METACALL_INVALID;
}

boolean metacall_value_to_bool(void * v)
{
	return value_to_bool(v);
}

char metacall_value_to_char(void * v)
{
	return value_to_char(v);
}

short metacall_value_to_short(void * v)
{
	return value_to_short(v);
}

int metacall_value_to_int(void * v)
{
	return value_to_int(v);
}

long metacall_value_to_long(void * v)
{
	return value_to_long(v);
}

float metacall_value_to_float(void * v)
{
	return value_to_float(v);
}

double metacall_value_to_double(void * v)
{
	return value_to_double(v);
}

char * metacall_value_to_string(void * v)
{
	return value_to_string(v);
}

void * metacall_value_to_ptr(void * v)
{
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

void * metacall_value_from_ptr(void * v, const void * ptr)
{
	return value_from_ptr(v, ptr);
}

void metacall_value_stringify(void * v, char * dest, size_t size, size_t * length)
{
	value_stringify(v, dest, size, length);
}

void metacall_value_destroy(void * v)
{
	value_destroy(v);
}
