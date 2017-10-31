/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <metacall_serial/metacall_serial_impl_deserialize.h>

#include <log/log.h>

#include <string.h>
#include <ctype.h>

/* -- Private Methods -- */

static int metacall_serial_impl_deserialize_bool(value * v, const char * src, size_t length);

static int metacall_serial_impl_deserialize_char(value * v, const char * src, size_t length);

static int metacall_serial_impl_deserialize_short(value * v, const char * src, size_t length);

static int metacall_serial_impl_deserialize_int(value * v, const char * src, size_t length);

static int metacall_serial_impl_deserialize_long(value * v, const char * src, size_t length);

static int metacall_serial_impl_deserialize_float(value * v, const char * src, size_t length);

static int metacall_serial_impl_deserialize_double(value * v, const char * src, size_t length);

static int metacall_serial_impl_deserialize_string(value * v, const char * src, size_t length);

static int metacall_serial_impl_deserialize_buffer(value * v, const char * src, size_t length);

static int metacall_serial_impl_deserialize_array(value * v, const char * src, size_t length);

static int metacall_serial_impl_deserialize_map(value * v, const char * src, size_t length);

static int metacall_serial_impl_deserialize_ptr(value * v, const char * src, size_t length);

/* -- Methods -- */

metacall_deserialize_impl_ptr metacall_serial_impl_deserialize_func(type_id id)
{
	static metacall_deserialize_impl_ptr deserialize_func[TYPE_SIZE] =
	{
		&metacall_serial_impl_deserialize_bool,
		&metacall_serial_impl_deserialize_char,
		&metacall_serial_impl_deserialize_short,
		&metacall_serial_impl_deserialize_int,
		&metacall_serial_impl_deserialize_long,
		&metacall_serial_impl_deserialize_float,
		&metacall_serial_impl_deserialize_double,
		&metacall_serial_impl_deserialize_string,
		&metacall_serial_impl_deserialize_buffer,
		&metacall_serial_impl_deserialize_array,
		&metacall_serial_impl_deserialize_map,
		&metacall_serial_impl_deserialize_ptr
	};

	return deserialize_func[id];
}

int metacall_serial_impl_deserialize_bool(value * v, const char * src, size_t length)
{
	static const char true_str[] = "true";

	static const char false_str[] = "false";

	if (strncmp(src, true_str, length) == 0)
	{
		*v = value_create_bool((boolean)1L);

		return (*v == NULL);
	}
	
	if (strncmp(src, false_str, length) == 0)
	{
		*v = value_create_bool((boolean)0L);

		return (*v == NULL);
	}

	return 1;
}

int metacall_serial_impl_deserialize_char(value * v, const char * src, size_t length)
{
	long l = 0;

	if (src[0] != '0' || src[1] != 'x')
	{
		return 1;
	}

	l = strtol(src, NULL, 16);

	if (l < 0 || l > 255)
	{
		return 1;
	}

	*v = value_create_char((char)(l & 0xFF));

	return (*v == NULL);
}

int metacall_serial_impl_deserialize_short(value * v, const char * src, size_t length)
{
	/* TODO */

	return 1;
}

int metacall_serial_impl_deserialize_int(value * v, const char * src, size_t length)
{
	size_t iterator;

	if (isdigit((int)src[0]) == 0 && src[0] != '-' && src[0] != '+')
	{
		return 1;
	}

	for (iterator = 1; iterator < length; ++iterator)
	{
		if (isdigit((int)src[iterator]) == 0)
		{
			return 1;
		}
	}

	*v = value_create_int(atoi(src));

	return (*v == NULL);
}

int metacall_serial_impl_deserialize_long(value * v, const char * src, size_t length)
{
	char buffer[24];

	const size_t last = length - 1;

	char * end = (char *)&src[last];

	size_t iterator;

	if (isdigit((int)src[0]) == 0 && src[0] != '-' && src[0] != '+')
	{
		return 1;
	}

	for (iterator = 1; iterator < last; ++iterator)
	{
		if (isdigit((int)src[iterator]) == 0)
		{
			return 1;
		}
	}

	if (*end != 'L')
	{
		return 1;
	}

	memcpy(buffer, src, last);

	buffer[last] = '\0';

	*v = value_create_long(strtol(buffer, NULL, 10));

	return (*v == NULL);
}

int metacall_serial_impl_deserialize_float(value * v, const char * src, size_t length)
{
	size_t iterator, is_digit;

	/* TODO: Improve parser */
	for (is_digit = 0, iterator = 0; iterator < length; ++iterator)
	{
		if (isdigit((int)src[iterator]) == 0 &&
			src[iterator] != '.' &&
			src[iterator] != '-' &&
			src[iterator] != '+' &&
			src[iterator] != 'E' &&
			src[iterator] != 'f')
		{
			return 1;
		}

		if (src[iterator] == '.')
		{
			if (is_digit == 1)
			{
				return 1;
			}

			is_digit = 1;
		}
	}

	/* TODO: Warning possible information lost in casting */
	*v = value_create_float((float)atof(src));

	return (*v == NULL);
}

int metacall_serial_impl_deserialize_double(value * v, const char * src, size_t length)
{
	size_t iterator, is_digit;

	/* TODO: Improve parser */
	for (is_digit = 0, iterator = 0; iterator < length; ++iterator)
	{
		if (isdigit((int)src[iterator]) == 0 &&
			src[iterator] != '.' &&
			src[iterator] != '-' &&
			src[iterator] != '+' &&
			src[iterator] != 'E')
		{
			return 1;
		}

		if (src[iterator] == '.')
		{
			if (is_digit == 1)
			{
				return 1;
			}

			is_digit = 1;
		}
	}

	*v = value_create_double(atof(src));

	return (*v == NULL);
}

int metacall_serial_impl_deserialize_string(value * v, const char * src, size_t length)
{
	size_t iterator;

	for (iterator = 0; iterator < length; ++iterator)
	{
		if (isalnum((int)src[iterator]) == 0)
		{
			return 1;
		}
	}

	*v = value_create_string(src, length);

	return (*v == NULL);
}

int metacall_serial_impl_deserialize_buffer(value * v, const char * src, size_t length)
{
	/* TODO */

	return 1;
}

int metacall_serial_impl_deserialize_array(value * v, const char * src, size_t length)
{
	/* TODO */

	return 1;
}

int metacall_serial_impl_deserialize_map(value * v, const char * src, size_t length)
{
	/* TODO */

	return 1;
}

int metacall_serial_impl_deserialize_ptr(value * v, const char * src, size_t length)
{
	/* TODO */

	return 1;
}
