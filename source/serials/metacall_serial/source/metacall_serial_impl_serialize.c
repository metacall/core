/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <metacall_serial/metacall_serial_impl_serialize.h>

#include <format/format_print.h>

#include <log/log.h>

/* -- Definitions -- */

#if defined(_WIN32) && defined(_MSC_VER)
#	define METACALL_SERIALIZE_VALUE_FORMAT_PTR "0x%p"
#elif defined(__linux) || defined(__linux__)
#	define METACALL_SERIALIZE_VALUE_FORMAT_PTR "%p"
#else
#	define METACALL_SERIALIZE_VALUE_FORMAT_PTR "%p"
#endif

/* -- Private Methods -- */

static void metacall_serial_impl_serialize_bool(value v, char * dest, size_t size, const char * format, size_t * length);

static void metacall_serial_impl_serialize_char(value v, char * dest, size_t size, const char * format, size_t * length);

static void metacall_serial_impl_serialize_short(value v, char * dest, size_t size, const char * format, size_t * length);

static void metacall_serial_impl_serialize_int(value v, char * dest, size_t size, const char * format, size_t * length);

static void metacall_serial_impl_serialize_long(value v, char * dest, size_t size, const char * format, size_t * length);

static void metacall_serial_impl_serialize_float(value v, char * dest, size_t size, const char * format, size_t * length);

static void metacall_serial_impl_serialize_double(value v, char * dest, size_t size, const char * format, size_t * length);

static void metacall_serial_impl_serialize_string(value v, char * dest, size_t size, const char * format, size_t * length);

static void metacall_serial_impl_serialize_buffer(value v, char * dest, size_t size, const char * format, size_t * length);

static void metacall_serial_impl_serialize_array(value v, char * dest, size_t size, const char * format, size_t * length);

static void metacall_serial_impl_serialize_map(value v, char * dest, size_t size, const char * format, size_t * length);

static void metacall_serial_impl_serialize_ptr(value v, char * dest, size_t size, const char * format, size_t * length);

static void metacall_serial_impl_serialize_null(value v, char * dest, size_t size, const char * format, size_t * length);

/* -- Methods -- */

const char * metacall_serial_impl_serialize_format(type_id id)
{
	static const char * metacall_serialize_format[TYPE_SIZE] =
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
		METACALL_SERIALIZE_VALUE_FORMAT_PTR,
		"%s"
	};

	return metacall_serialize_format[id];
}

metacall_serialize_impl_ptr metacall_serial_impl_serialize_func(type_id id)
{
	static metacall_serialize_impl_ptr serialize_func[TYPE_SIZE] =
	{
		&metacall_serial_impl_serialize_bool,
		&metacall_serial_impl_serialize_char,
		&metacall_serial_impl_serialize_short,
		&metacall_serial_impl_serialize_int,
		&metacall_serial_impl_serialize_long,
		&metacall_serial_impl_serialize_float,
		&metacall_serial_impl_serialize_double,
		&metacall_serial_impl_serialize_string,
		&metacall_serial_impl_serialize_buffer,
		&metacall_serial_impl_serialize_array,
		&metacall_serial_impl_serialize_map,
		&metacall_serial_impl_serialize_ptr,
		&metacall_serial_impl_serialize_null
	};

	return serialize_func[id];
}

void metacall_serial_impl_serialize_bool(value v, char * dest, size_t size, const char * format, size_t * length)
{
	static const char value_boolean_str[] = "false\0true";

	int offset = sizeof("false") * value_to_bool(v);

	if (offset < 0 || (size_t)offset >= sizeof(value_boolean_str))
	{
		offset = 0;
	}

	*length = snprintf(dest, size, format, (const char *)(&value_boolean_str[offset]));
}

void metacall_serial_impl_serialize_char(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_char(v));
}

void metacall_serial_impl_serialize_short(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_short(v));
}

void metacall_serial_impl_serialize_int(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_int(v));
}

void metacall_serial_impl_serialize_long(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_long(v));
}

void metacall_serial_impl_serialize_float(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_float(v));
}

void metacall_serial_impl_serialize_double(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_double(v));
}

void metacall_serial_impl_serialize_string(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_string(v));
}

void metacall_serial_impl_serialize_buffer(value v, char * dest, size_t size, const char * format, size_t * length)
{
	if (dest == NULL && size == 0)
	{
		/* Each byte is going to be printed as two chars */
		*length = (value_type_size(v) * 2);
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

void metacall_serial_impl_serialize_array(value v, char * dest, size_t size, const char * format, size_t * length)
{
	size_t iterator, array_value_length = 0, array_size = value_type_size(v) / sizeof(const value);

	value * array_value = value_to_array(v);

	(void)format;

	/* Calculate sum of all array values lenght */
	for (iterator = 0; iterator < array_size; ++iterator)
	{
		value current_value = array_value[iterator];

		type_id id = value_type_id(current_value);

		const char * format = metacall_serial_impl_serialize_format(id);

		metacall_serialize_impl_ptr serialize_ptr = metacall_serial_impl_serialize_func(id);

		size_t length = 0;

		serialize_ptr(current_value, NULL, 0, format, &length);

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

			const char * format = metacall_serial_impl_serialize_format(id);

			metacall_serialize_impl_ptr serialize_ptr = metacall_serial_impl_serialize_func(id);

			size_t length = 0;

			serialize_ptr(current_value, &dest[array_value_length_current], array_value_length - array_value_length_current, format, &length);

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

void metacall_serial_impl_serialize_map(value v, char * dest, size_t size, const char * format, size_t * length)
{
	/* TODO: Implement map iteration like array stringify */

	*length = snprintf(dest, size, format, value_to_map(v));
}

void metacall_serial_impl_serialize_ptr(value v, char * dest, size_t size, const char * format, size_t * length)
{
	*length = snprintf(dest, size, format, value_to_ptr(v));
}

void metacall_serial_impl_serialize_null(value v, char * dest, size_t size, const char * format, size_t * length)
{
	static const char value_null_str[] = "(null)";

	(void)v;

	*length = snprintf(dest, size, format, value_null_str);
}
