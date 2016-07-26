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

value value_create_ptr(void * p)
{
	return value_create(&p, sizeof(void *));
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

void * value_to_ptr(value v)
{
	void * p = NULL;

	value_to(v, &p, sizeof(void *));

	return p;
}

void value_from(value v, const void * data, size_t bytes)
{
	if (v != NULL && data != NULL && bytes > 0)
	{
		void * dest = value_data(v);

		memcpy(dest, data, bytes);
	}
}

void value_from_char(value v, char c)
{
	value_from(v, &c, sizeof(char));
}

void value_from_int(value v, int i)
{
	value_from(v, &i, sizeof(int));
}

void value_from_long(value v, long l)
{
	value_from(v, &l, sizeof(long));
}

void value_from_double(value v, double d)
{
	value_from(v, &d, sizeof(double));
}

void value_from_ptr(value v, void * p)
{
	value_from(v, &p, sizeof(void *));
}

void value_destroy(value v)
{
	if (v != NULL)
	{
		value_impl impl = value_get_impl(v);

		free(impl);
	}
}
