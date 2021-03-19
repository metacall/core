/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <reflect/reflect_value.h>

#include <stdint.h>
#include <string.h>

/* -- Forward Declarations -- */

struct value_impl_type;

/* -- Type Definitions -- */

typedef struct value_impl_type *value_impl;

/* -- Member Data -- */

struct value_impl_type
{
	size_t bytes;
	size_t ref_count;
	value_finalizer_cb finalizer;
	void *finalizer_data;
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
	impl->finalizer = NULL;
	impl->finalizer_data = NULL;

	return (value)(((uintptr_t)impl) + sizeof(struct value_impl_type));
}

value value_create(const void *data, size_t bytes)
{
	value v = value_alloc(bytes);

	if (v == NULL)
	{
		return NULL;
	}

	memcpy(v, data, bytes);

	return v;
}

value value_copy(value v)
{
	size_t size = value_size(v);

	value copy = value_alloc(size);

	if (copy == NULL)
	{
		return NULL;
	}

	memcpy(copy, value_data(v), size);

	return copy;
}

void value_move(value src, value dst)
{
	if (src != NULL && dst != NULL)
	{
		value_impl impl_src = value_descriptor(src);
		value_impl impl_dst = value_descriptor(dst);

		impl_dst->finalizer = impl_src->finalizer;
		impl_dst->finalizer_data = impl_src->finalizer_data;

		impl_src->finalizer = NULL;
		impl_src->finalizer_data = NULL;
	}
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

void value_finalizer(value v, value_finalizer_cb finalizer, void *finalizer_data)
{
	value_impl impl = value_descriptor(v);

	if (impl != NULL)
	{
		impl->finalizer = finalizer;
		impl->finalizer_data = finalizer_data;
	}
}

void *value_data(value v)
{
	if (v == NULL)
	{
		return NULL;
	}

	return v;
}

void value_to(value v, void *data, size_t bytes)
{
	void *src = value_data(v);

	if (src != NULL && data != NULL && bytes > 0)
	{
		memcpy(data, src, bytes);
	}
}

value value_from(value v, const void *data, size_t bytes)
{
	void *dest = value_data(v);

	if (dest != NULL && bytes > 0)
	{
		if (data != NULL)
		{
			memcpy(dest, data, bytes);
		}
		else
		{
			memset(dest, 0, bytes);
		}
	}

	return v;
}

void value_destroy(value v)
{
	value_impl impl = value_descriptor(v);

	if (impl != NULL && impl->ref_count <= 1)
	{
		if (impl->finalizer != NULL)
		{
			impl->finalizer(v, impl->finalizer_data);
		}

		free(impl);
	}
}
