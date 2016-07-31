/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
*
*/

/* -- Headers -- */

#include <reflect/value.h>

#include <string.h>

/* -- Forward Declarations -- */

struct value_impl_type;

/* -- Type Definitions -- */

typedef struct value_impl_type * value_impl;

/* -- Member Data -- */

struct value_impl_type
{
	size_t bytes;
	size_t ref_count;
};

/* -- Private Methods -- */

/**
*  @brief
*    Access to the descriptor of a value
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Pointer to the header of a value
*/
value_impl value_get_impl(value v);

/* -- Methods -- */

value_impl value_get_impl(value v)
{
	if (v != NULL)
	{
		return (value_impl)((char *)v - sizeof(struct value_impl_type));
	}

	return NULL;
}

value value_create(const void * data, size_t bytes)
{
	value_impl impl = malloc(sizeof(struct value_impl_type) + bytes);

	if (impl != NULL)
	{
		value dest = (value)((char *)impl + sizeof(struct value_impl_type));

		impl->bytes = bytes;

		impl->ref_count = 1;
		
		memcpy(dest, data, bytes);

		return dest;
	}

	return NULL;
}

value value_create_bool(boolean b)
{
	return value_create(&b, sizeof(boolean));
}

value value_create_char(char c)
{
	return value_create(&c, sizeof(char));
}

value value_create_int(int i)
{
	return value_create(&i, sizeof(int));
}

value value_create_long(long l)
{
	return value_create(&l, sizeof(long));
}

value value_create_double(double d)
{
	return value_create(&d, sizeof(double));
}

value value_create_string(const char * str, size_t length)
{
	if (str != NULL && length > 0)
	{
		size_t bytes = length + 1;

		value v = value_create(str, bytes);

		if (v != NULL)
		{
			memcpy(v, str, bytes);
		}

		return v;
	}

	return NULL;
}

value value_create_ptr(const void * ptr)
{
	return value_create((const void *)&ptr, sizeof(const void *));
}

size_t value_size(value v)
{
	if (v != NULL)
	{
		value_impl impl = value_get_impl(v);

		return impl->bytes;
	}

	return 0;
}

void value_ref_inc(value v)
{
	if (v != NULL)
	{
		value_impl impl = value_get_impl(v);

		++impl->ref_count;
	}
}

void value_ref_dec(value v)
{
	if (v != NULL)
	{
		value_impl impl = value_get_impl(v);

		--impl->ref_count;

		if (impl->ref_count == 0)
		{
			value_destroy(v);
		}
	}
}

void * value_data(value v)
{
	if (v != NULL)
	{
		return v;
	}

	return NULL;
}

void value_to(value v, void * data, size_t bytes)
{
	if (v != NULL && data != NULL && bytes > 0)
	{
		void * src = value_data(v);

		memcpy(data, src, bytes);
	}
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
	return value_data(v);
}

value value_from(value v, const void * data, size_t bytes)
{
	if (v != NULL && data != NULL && bytes > 0)
	{
		void * dest = value_data(v);

		memcpy(dest, data, bytes);
	}

	return v;
}

value value_from_bool(value v, boolean b)
{
	return value_from(v, &b, sizeof(boolean));
}

value value_from_char(value v, char c)
{
	return value_from(v, &c, sizeof(char));
}

value value_from_int(value v, int i)
{
	return value_from(v, &i, sizeof(int));
}

value value_from_long(value v, long l)
{
	return value_from(v, &l, sizeof(long));
}

value value_from_double(value v, double d)
{
	return value_from(v, &d, sizeof(double));
}

value value_from_string(value v, const char * str, size_t length)
{
	if (v != NULL && str != NULL && length > 0)
	{
		value_impl impl = value_get_impl(v);

		size_t bytes = length + 1;

		size_t size = (bytes <= impl->bytes) ? bytes : impl->bytes;

		return value_from(v, str, size);
	}

	return v;
}

value value_from_ptr(value v, const void * ptr)
{
	return value_from(v, &ptr, sizeof(const void *));
}

void value_destroy(value v)
{
	if (v != NULL)
	{
		value_impl impl = value_get_impl(v);

		if (impl->ref_count <= 1)
		{
			free(impl);
		}
	}
}
