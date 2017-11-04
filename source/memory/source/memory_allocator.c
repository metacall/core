/*
 *	Memory Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic cross-platform memory utility.
 *
 */

/* -- Headers -- */

#include <memory/memory.h>

/* -- Member Data -- */

struct memory_allocator_type
{
	memory_allocator_iface iface;
	memory_allocator_impl impl;

	struct
	{
		size_t alloc;
		size_t bad_alloc;
		size_t realloc;
		size_t bad_realloc;
		size_t dealloc;
	} size;
};

/* -- Methods -- */

memory_allocator memory_allocator_create(memory_allocator_iface iface, void * ctx)
{
	memory_allocator allocator;

	memory_allocator_impl impl = iface->create(ctx);

	if (impl == NULL)
	{
		return NULL;
	}

	allocator = iface->allocate(impl, sizeof(struct memory_allocator_type));

	if (allocator == NULL)
	{
		iface->destroy(impl);

		return NULL;
	}

	allocator->iface->create = iface->create;
	allocator->iface->allocate = iface->allocate;
	allocator->iface->deallocate = iface->deallocate;
	allocator->iface->destroy = iface->destroy;
	allocator->impl = impl;

	allocator->size.alloc = 0;
	allocator->size.bad_alloc = 0;
	allocator->size.realloc = 0;
	allocator->size.bad_realloc = 0;
	allocator->size.dealloc = 0;

	return allocator;
}

void * memory_allocator_allocate(memory_allocator allocator, size_t size)
{
	void * data = allocator->iface->allocate(allocator->impl, size);

	if (data == NULL)
	{
		++allocator->size.bad_alloc;
	}
	else
	{
		++allocator->size.alloc;
	}

	return data;
}

void * memory_allocator_reallocate(memory_allocator allocator, void * data, size_t size)
{
	void * new_data = allocator->iface->reallocate(allocator->impl, data, size);

	if (new_data == NULL)
	{
		++allocator->size.bad_realloc;
	}
	else
	{
		++allocator->size.realloc;
	}

	return new_data;
}

void memory_allocator_deallocate(memory_allocator allocator, void * data)
{
	allocator->iface->deallocate(allocator->impl, data);

	++allocator->size.dealloc;
}

size_t memory_allocator_used(memory_allocator allocator)
{
	/* TODO: Implement implicit set to hold pointer to size mapping and global byte count*/
	(void)allocator;

	return 0;
}

void memory_allocator_destroy(memory_allocator allocator)
{
	memory_allocator_impl impl = allocator->impl;

	memory_allocator_destroy_ptr destroy = allocator->iface->destroy;

	allocator->iface->deallocate(allocator, allocator);

	destroy(impl);
}
