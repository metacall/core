/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

#include <metacall/metacall_value.h>

#include <reflect/reflect_value.h>
#include <reflect/reflect_value_type.h>

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

void * metacall_value_create_buffer(const void * buffer, size_t size)
{
	return value_create_buffer(buffer, size);
}

void * metacall_value_create_array(const void * values[], size_t size)
{
	return value_create_array((const value *)values, size);
}

void * metacall_value_create_map(const void * tuples[], size_t size)
{
	return value_create_map((const value *)tuples, size);
}

void * metacall_value_create_ptr(const void * ptr)
{
	return value_create_ptr(ptr);
}

void * metacall_value_create_null()
{
	return value_create_null();
}

size_t metacall_value_size(void * v)
{
	return value_type_size(v);
}

size_t metacall_value_count(void * v)
{
	return value_type_count(v);
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
			METACALL_BUFFER,
			METACALL_ARRAY,
			METACALL_MAP,
			METACALL_PTR,
			METACALL_NULL,

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
			((int)TYPE_BUFFER == (int)METACALL_BUFFER) &&
			((int) TYPE_ARRAY == (int) METACALL_ARRAY) &&
			((int) TYPE_MAP == (int) METACALL_MAP) &&
			((int) TYPE_PTR == (int) METACALL_PTR) &&
			((int) TYPE_NULL == (int) METACALL_NULL) &&
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

void * metacall_value_to_buffer(void * v)
{
	assert(value_type_id(v) == TYPE_BUFFER);

	return value_to_buffer(v);
}

void ** metacall_value_to_array(void * v)
{
	assert(value_type_id(v) == TYPE_ARRAY);

	return value_to_array(v);
}

void ** metacall_value_to_map(void * v)
{
	assert(value_type_id(v) == TYPE_MAP);

	return value_to_map(v);
}

void * metacall_value_to_ptr(void * v)
{
	assert(value_type_id(v) == TYPE_PTR);

	return value_to_ptr(v);
}

void * metacall_value_to_null(void * v)
{
	assert(value_type_id(v) == TYPE_NULL);

	return value_to_null(v);
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

void * metacall_value_from_buffer(void * v, const void * buffer, size_t size)
{
	return value_from_buffer(v, buffer, size);
}

void * metacall_value_from_array(void * v, const void * values[], size_t size)
{
	return value_from_array(v, (const value *)values, size);
}

void * metacall_value_from_map(void * v, const void * tuples[], size_t size)
{
	return value_from_map(v, (const value *)tuples, size);
}

void * metacall_value_from_ptr(void * v, const void * ptr)
{
	return value_from_ptr(v, ptr);
}

void * metacall_value_from_null(void * v)
{
	return value_from_null(v);
}

boolean metacall_value_cast_bool(void ** v)
{
	if (value_type_id(*v) != TYPE_BOOL)
	{
		value v_cast = value_type_cast(*v, TYPE_BOOL);

		if (v_cast != NULL)
		{
			*v = v_cast;
		}
	}

	return value_to_bool(*v);
}

char metacall_value_cast_char(void ** v)
{
	if (value_type_id(*v) != TYPE_CHAR)
	{
		value v_cast = value_type_cast(*v, TYPE_CHAR);

		if (v_cast != NULL)
		{
			*v = v_cast;
		}
	}

	return value_to_char(*v);
}

short metacall_value_cast_short(void ** v)
{
	if (value_type_id(*v) != TYPE_SHORT)
	{
		value v_cast = value_type_cast(*v, TYPE_SHORT);

		if (v_cast != NULL)
		{
			*v = v_cast;
		}
	}

	return value_to_short(*v);
}

int metacall_value_cast_int(void ** v)
{
	if (value_type_id(*v) != TYPE_INT)
	{
		value v_cast = value_type_cast(*v, TYPE_INT);

		if (v_cast != NULL)
		{
			*v = v_cast;
		}
	}

	return value_to_int(*v);
}

long metacall_value_cast_long(void ** v)
{
	if (value_type_id(*v) != TYPE_LONG)
	{
		value v_cast = value_type_cast(*v, TYPE_LONG);

		if (v_cast != NULL)
		{
			*v = v_cast;
		}
	}

	return value_to_long(*v);
}

float metacall_value_cast_float(void ** v)
{
	if (value_type_id(*v) != TYPE_FLOAT)
	{
		value v_cast = value_type_cast(*v, TYPE_FLOAT);

		if (v_cast != NULL)
		{
			*v = v_cast;
		}
	}

	return value_to_float(*v);
}

double metacall_value_cast_double(void ** v)
{
	if (value_type_id(*v) != TYPE_DOUBLE)
	{
		value v_cast = value_type_cast(*v, TYPE_DOUBLE);

		if (v_cast != NULL)
		{
			*v = v_cast;
		}
	}

	return value_to_double(*v);
}

char * metacall_value_cast_string(void ** v)
{
	if (value_type_id(*v) != TYPE_STRING)
	{
		value v_cast = value_type_cast(*v, TYPE_STRING);

		if (v_cast != NULL)
		{
			*v = v_cast;
		}
	}

	return value_to_string(*v);
}

void * metacall_value_cast_buffer(void ** v)
{
	if (value_type_id(*v) != TYPE_BUFFER)
	{
		value v_cast = value_type_cast(*v, TYPE_BUFFER);

		if (v_cast != NULL)
		{
			*v = v_cast;
		}
	}

	return value_to_buffer(*v);
}

void ** metacall_value_cast_array(void ** v)
{
	if (value_type_id(*v) != TYPE_ARRAY)
	{
		value v_cast = value_type_cast(*v, TYPE_ARRAY);

		if (v_cast != NULL)
		{
			*v = v_cast;
		}
	}

	return value_to_array(*v);
}

void * metacall_value_cast_map(void ** v)
{
	if (value_type_id(*v) != TYPE_MAP)
	{
		value v_cast = value_type_cast(*v, TYPE_MAP);

		if (v_cast != NULL)
		{
			*v = v_cast;
		}
	}

	return value_to_map(*v);
}

void * metacall_value_cast_ptr(void ** v)
{
	if (value_type_id(*v) != TYPE_PTR)
	{
		value v_cast = value_type_cast(*v, TYPE_PTR);

		if (v_cast != NULL)
		{
			*v = v_cast;
		}
	}

	return value_to_ptr(*v);
}

void * metacall_value_cast_null(void ** v)
{
	if (value_type_id(*v) != TYPE_NULL)
	{
		value v_cast = value_type_cast(*v, TYPE_NULL);

		if (v_cast != NULL)
		{
			*v = v_cast;
		}
	}

	return value_to_null(*v);
}

void metacall_value_destroy(void * v)
{
	value_type_destroy(v);
}
