/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
*
*/

/* -- Headers -- */

#include <reflect/reflect_value_type.h>

#include <format/format_print.h>

/* -- Type Definitions -- */

typedef void (*value_stringify_impl_ptr)(value, char *, size_t, const char *, size_t *);

/* -- Private Methods -- */

/**
*  @brief
*    Provides value format depending on type id @id
*
*  @param[in] id
*    Type of wanted format
*
*  @return
*    Print format assigned to type id @id
*/
static const char * value_stringify_format(type_id id);

/**
*  @brief
*    Provides pointer to function for printing depending on type id @id
*
*  @param[in] id
*    Type of wanted stringify function pointer
*
*  @return
*    Stringify function pointer assigned to type id @id
*/
static value_stringify_impl_ptr value_stringify_impl(type_id id);

static void value_stringify_bool(value v, char * dest, size_t size, const char * format, size_t * length);

static void value_stringify_char(value v, char * dest, size_t size, const char * format, size_t * length);

static void value_stringify_short(value v, char * dest, size_t size, const char * format, size_t * length);

static void value_stringify_int(value v, char * dest, size_t size, const char * format, size_t * length);

static void value_stringify_long(value v, char * dest, size_t size, const char * format, size_t * length);

static void value_stringify_double(value v, char * dest, size_t size, const char * format, size_t * length);

static void value_stringify_string(value v, char * dest, size_t size, const char * format, size_t * length);

static void value_stringify_ptr(value v, char * dest, size_t size, const char * format, size_t * length);

/* -- Methods -- */

static const char * value_stringify_format(type_id id)
{
	static const char * value_format[TYPE_SIZE] =
	{
		"%s",
		"%c",
		"%d",
		"%d",
		"%ld",
		"%.6f",
		"%s",
		#if defined(_WIN32) && defined(_MSC_VER)
			"0x%p"
		#elif defined(__linux) || defined(__linux__)
			"%p"
		#else
			"%p"
		#endif
	};

	return value_format[id];
}

static value_stringify_impl_ptr value_stringify_impl(type_id id)
{
	static value_stringify_impl_ptr stringify_ptr[TYPE_SIZE] =
	{
		&value_stringify_bool,
		&value_stringify_char,
		&value_stringify_short,
		&value_stringify_int,
		&value_stringify_long,
		&value_stringify_double,
		&value_stringify_string,
		&value_stringify_ptr
	};

	return stringify_ptr[id];
}

static void value_stringify_bool(value v, char * dest, size_t size, const char * format, size_t * length)
{
	static const char value_boolean_str[] = "false\0true";

	int offset = sizeof("false") * value_to_bool(v);

	if (offset < 0 || (size_t)offset >= sizeof(value_boolean_str))
	{
		offset = 0;
	}

	*length = snprintf(dest, size, format, (const char *)(&value_boolean_str[offset]));
}

static void value_stringify_char(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_char(v));
}

static void value_stringify_short(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_short(v));
}

static void value_stringify_int(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_int(v));
}

static void value_stringify_long(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_long(v));
}

static void value_stringify_double(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_double(v));
}

static void value_stringify_string(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_string(v));
}

static void value_stringify_ptr(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_ptr(v));
}

void value_stringify(value v, char * dest, size_t size, size_t * length)
{
	type_id id = value_type_id(v);

	const char * format = value_stringify_format(id);

	value_stringify_impl_ptr stringify_ptr = value_stringify_impl(id);

	stringify_ptr(v, dest, size, format, length);
}
