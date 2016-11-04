/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
*
*/

/* -- Headers -- */

#include <reflect/reflect_value.h>

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
value_impl value_descriptor(value v);

/* -- Methods -- */

value_impl value_descriptor(value v)
{
	if (v != NULL)
	{
		return (value_impl)((char *)v - sizeof(struct value_impl_type));
	}

	return NULL;
}

value value_alloc(size_t bytes)
{
	return malloc(sizeof(struct value_impl_type) + bytes);
}

value value_create(const void * data, size_t bytes)
{
	value_impl impl = value_alloc(bytes);

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

size_t value_size(value v)
{
	if (v != NULL)
	{
		value_impl impl = value_descriptor(v);

		return impl->bytes;
	}

	return 0;
}

void value_ref_inc(value v)
{
	if (v != NULL)
	{
		value_impl impl = value_descriptor(v);

		++impl->ref_count;
	}
}

void value_ref_dec(value v)
{
	if (v != NULL)
	{
		value_impl impl = value_descriptor(v);

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

value value_from(value v, const void * data, size_t bytes)
{
	if (v != NULL && data != NULL && bytes > 0)
	{
		void * dest = value_data(v);

		memcpy(dest, data, bytes);
	}

	return v;
}

void value_destroy(value v)
{
	if (v != NULL)
	{
		value_impl impl = value_descriptor(v);

		if (impl->ref_count <= 1)
		{
			free(impl);
		}
	}
}
