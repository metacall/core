/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <metacall_serial/metacall_serial_impl.h>
#include <metacall_serial/metacall_serial_impl_serialize.h>
#include <metacall_serial/metacall_serial_impl_deserialize.h>

#include <log/log.h>

/* -- Private Methods -- */

static void metacall_serial_impl_serialize_value(value v, char * dest, size_t size, size_t * length);

static value metacall_serial_impl_deserialize_value(const char * buffer, size_t size);

/* -- Methods -- */

const char * metacall_serial_impl_extension()
{
	static const char extension[] = "meta";

	return extension;
}

serial_impl_handle metacall_serial_impl_initialize(memory_allocator allocator, serial_host host)
{
	log_copy(host->log);

	return allocator;
}

void metacall_serial_impl_serialize_value(value v, char * dest, size_t size, size_t * length)
{
	type_id id = value_type_id(v);

	const char * format = metacall_serial_impl_serialize_format(id);

	metacall_serialize_impl_ptr serialize_ptr = metacall_serial_impl_serialize_func(id);

	serialize_ptr(v, dest, size, format, length);
}

char * metacall_serial_impl_serialize(serial_impl_handle handle, value v, size_t * size)
{
	memory_allocator allocator;

	size_t length, buffer_size;

	char * buffer;

	if (handle == NULL || v == NULL || size == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Serialization called with wrong arguments in MetaCall Native Format implementation");

		return NULL;
	}

	allocator = (memory_allocator)handle;

	metacall_serial_impl_serialize_value(v, NULL, 0, &length);

	if (length == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Serialization invalid length calculation in MetaCall Native Format implementation");

		return NULL;
	}

	buffer_size = length + 1;

	buffer = memory_allocator_allocate(allocator, sizeof(char) * (buffer_size));

	if (buffer == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Serialization invalid buffer allocation in MetaCall Native Format implementation");

		*size = 0;

		return NULL;
	}

	metacall_serial_impl_serialize_value(v, buffer, buffer_size, &length);

	if (length + 1 != buffer_size)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Serialization invalid length + 1 != buffer "
			"(%" PRIuS " != %" PRIuS ") in MetaCall Native Format implementation", length + 1, buffer_size);
		
		memory_allocator_deallocate(allocator, buffer);

		*size = 0;

		return NULL;
	}

	*size = buffer_size;

	return buffer;
}

value metacall_serial_impl_deserialize_value(const char * buffer, size_t size)
{
	value v = NULL;

	type_id id;

	for (id = 0; id < TYPE_SIZE; ++id)
	{
		metacall_deserialize_impl_ptr deserialize_ptr = metacall_serial_impl_deserialize_func(id);

		if (deserialize_ptr(&v, buffer, size) == 0)
		{
			return v;
		}
	}

	log_write("metacall", LOG_LEVEL_ERROR, "Deserialization unsuported value type in MetaCall Native Format implementation");

	return NULL;
}

value metacall_serial_impl_deserialize(serial_impl_handle handle, const char * buffer, size_t size)
{
	if (handle == NULL || buffer == NULL || size == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Deserialization called with wrong arguments in MetaCall Native Format implementation");

		return NULL;
	}

	return metacall_serial_impl_deserialize_value(buffer, size);
}

int metacall_serial_impl_destroy(serial_impl_handle handle)
{
	(void)handle;

	return 0;
}
