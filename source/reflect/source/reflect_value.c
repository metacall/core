/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
*
*/

/* -- Headers -- */

#include <reflect/reflect_value.h>

#include <stdint.h>
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
	if (v == NULL)
	{
		return NULL;
	}

	return (value_impl)(((uintptr_t)v) - sizeof(struct value_impl_type));
}

value value_alloc(size_t bytes)
{
	value_impl impl = malloc(sizeof(struct value_impl_type) + bytes);

	if (impl == NULL)
	{
		return NULL;
	}

	impl->bytes = bytes;

	impl->ref_count = 1;

	return (value)(((uintptr_t)impl) + sizeof(struct value_impl_type));
}

value value_create(const void * data, size_t bytes)
{
	value v = value_alloc(bytes);

	if (v == NULL)
	{
		return NULL;
	}

	memcpy(v, data, bytes);

	return v;
}

size_t value_size(value v)
{
	value_impl impl = value_descriptor(v);

	if (impl == NULL)
	{
		return 0;
	}

	return impl->bytes;
}

void value_ref_inc(value v)
{
	value_impl impl = value_descriptor(v);

	if (impl != NULL)
	{
		++impl->ref_count;
	}
}

void value_ref_dec(value v)
{
	value_impl impl = value_descriptor(v);

	if (impl != NULL)
	{
		--impl->ref_count;

		if (impl->ref_count == 0)
		{
			value_destroy(v);
		}
	}
}

void * value_data(value v)
{
	if (v == NULL)
	{
		return NULL;
	}

	return v;
}

void value_to(value v, void * data, size_t bytes)
{
	void * src = value_data(v);

	if (src != NULL && data != NULL && bytes > 0)
	{
		memcpy(data, src, bytes);
	}
}

value value_from(value v, const void * data, size_t bytes)
{
	void * dest = value_data(v);

	if (dest != NULL && data != NULL && bytes > 0)
	{
		memcpy(dest, data, bytes);
	}

	return v;
}

void value_destroy(value v)
{
	value_impl impl = value_descriptor(v);

	if (impl != NULL && impl->ref_count <= 1)
	{
		free(impl);
	}
}
