/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
*
*/

/* -- Headers -- */

#include <reflect/reflect_value_type.h>
#include <reflect/reflect_value_type_stringify.h>

#include <format/format_print.h>

#include <log/log.h>

/* -- Definitions -- */

#if defined(_WIN32) && defined(_MSC_VER)
#	define VALUE_TYPE_STRINGIFY_FORMAT_PTR "0x%p"
#elif defined(__linux) || defined(__linux__)
#	define VALUE_TYPE_STRINGIFY_FORMAT_PTR "%p"
#else
#	define VALUE_TYPE_STRINGIFY_FORMAT_PTR "%p"
#endif

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

static void value_stringify_float(value v, char * dest, size_t size, const char * format, size_t * length);

static void value_stringify_double(value v, char * dest, size_t size, const char * format, size_t * length);

static void value_stringify_string(value v, char * dest, size_t size, const char * format, size_t * length);

static void value_stringify_buffer(value v, char * dest, size_t size, const char * format, size_t * length);

static void value_stringify_array(value v, char * dest, size_t size, const char * format, size_t * length);

static void value_stringify_map(value v, char * dest, size_t size, const char * format, size_t * length);

static void value_stringify_ptr(value v, char * dest, size_t size, const char * format, size_t * length);

/* -- Methods -- */

const char * value_stringify_format(type_id id)
{
	static const char * value_format[TYPE_SIZE] =
	{
		"%s",
		"%c",
		"%d",
		"%d",
		"%ld",
		"%.6ff",
		"%.6f",
		"%s",
		"%02x",
		NULL, /* Unused */
		NULL, /* Unused */
		VALUE_TYPE_STRINGIFY_FORMAT_PTR
	};

	return value_format[id];
}

value_stringify_impl_ptr value_stringify_impl(type_id id)
{
	static value_stringify_impl_ptr stringify_ptr[TYPE_SIZE] =
	{
		&value_stringify_bool,
		&value_stringify_char,
		&value_stringify_short,
		&value_stringify_int,
		&value_stringify_long,
		&value_stringify_float,
		&value_stringify_double,
		&value_stringify_string,
		&value_stringify_buffer,
		&value_stringify_array,
		&value_stringify_map,
		&value_stringify_ptr
	};

	return stringify_ptr[id];
}

void value_stringify_bool(value v, char * dest, size_t size, const char * format, size_t * length)
{
	static const char value_boolean_str[] = "false\0true";

	int offset = sizeof("false") * value_to_bool(v);

	if (offset < 0 || (size_t)offset >= sizeof(value_boolean_str))
	{
		offset = 0;
	}

	*length = snprintf(dest, size, format, (const char *)(&value_boolean_str[offset]));
}

void value_stringify_char(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_char(v));
}

void value_stringify_short(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_short(v));
}

void value_stringify_int(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_int(v));
}

void value_stringify_long(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_long(v));
}

void value_stringify_float(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_float(v));
}

void value_stringify_double(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_double(v));
}

void value_stringify_string(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_string(v));
}

void value_stringify_buffer(value v, char * dest, size_t size, const char * format, size_t * length)
{
	if (dest == NULL && size == 0)
	{
		/* Each byte is going to be printed as two chars */
		*length = (value_type_size(v) * 2) - 1;
	}
	else
	{
		const char * buffer = (const char *)value_to_buffer(v);

		size_t dest_iterator, iterator, buffer_length = 0, buffer_size = value_type_size(v);

		for (iterator = 0, dest_iterator = 0; iterator < buffer_size && dest_iterator < size; ++iterator, dest_iterator += 2)
		{
			buffer_length += snprintf(&dest[dest_iterator], size - dest_iterator, format, buffer[iterator]);
		}

		*length = buffer_length;
	}
}

void value_stringify_array(value v, char * dest, size_t size, const char * format, size_t * length)
{
	size_t iterator, array_value_length = 0, array_size = value_type_size(v) / sizeof(const value);

	value * array_value = value_to_array(v);

	(void)format;

	/* Calculate sum of all array values lenght */
	for (iterator = 0; iterator < array_size; ++iterator)
	{
		value current_value = array_value[iterator];

		type_id id = value_type_id(current_value);

		const char * format = value_stringify_format(id);

		value_stringify_impl_ptr stringify_ptr = value_stringify_impl(id);

		size_t length = 0;

		stringify_ptr(current_value, NULL, 0, format, &length);

		array_value_length += length;
	}

	/* Add length of parethesis and comas */
	array_value_length += 2 + (array_size - 1);

	if (dest == NULL && size == 0)
	{
		/* Return length if no destination available */
		*length = array_value_length;
	}
	else
	{
		/* Stringify all values */
		size_t array_value_length_current = 0;

		if (array_value_length >= size)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Not enough space for value array stringification need %" PRIuS " bytes", array_value_length - size + 1);

			*length = 0;

			return;
		}

		dest[array_value_length_current++] = '[';

		for (iterator = 0; iterator < array_size; ++iterator)
		{
			value current_value = array_value[iterator];

			type_id id = value_type_id(current_value);

			const char * format = value_stringify_format(id);

			value_stringify_impl_ptr stringify_ptr = value_stringify_impl(id);

			size_t length = 0;

			stringify_ptr(current_value, &dest[array_value_length_current], array_value_length - array_value_length_current, format, &length);

			array_value_length_current += length;

			if (iterator < array_size - 1)
			{
				dest[array_value_length_current++] = ',';
			}
		}

		dest[array_value_length_current++] = ']';

		dest[array_value_length_current] = '\0';

		if (array_value_length_current != array_value_length)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid string length calculation in value array stringification");
		}

		*length = array_value_length;
	}
}

void value_stringify_map(value v, char * dest, size_t size, const char * format, size_t * length)
{
	/* TODO: Implement map iteration like array stringify */

	*length = snprintf(dest, size, format, value_to_map(v));
}

void value_stringify_ptr(value v, char * dest, size_t size, const char * format, size_t * length)
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

value value_type_stringify(value v)
{
	type_id id = value_type_id(v);

	const char * format = value_stringify_format(id);

	value_stringify_impl_ptr stringify_ptr = value_stringify_impl(id);

	size_t length, size;

	value result;

	stringify_ptr(v, NULL, 0, format, &length);

	if (length == 0)
	{
		return NULL;
	}

	size = length + 1;

	result = value_type_create(NULL, size, TYPE_STRING);

	if (result == NULL)
	{
		return NULL;
	}

	stringify_ptr(v, value_data(result), size, format, &length);

	if (length + 1 != size)
	{
		value_destroy(result);
	}

	value_destroy(v);

	return result;
}
