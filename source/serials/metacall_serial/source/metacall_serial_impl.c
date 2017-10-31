/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <metacall_serial/metacall_serial_impl.h>
#include <metacall_serial/metacall_serial_impl_serialize.h>

#include <log/log.h>

/* -- Private Methods -- */

static void metacall_serial_impl_serialize_value(value v, char * dest, size_t size, size_t * length);

/* -- Methods -- */

const char * metacall_serial_impl_extension()
{
	static const char extension[] = "meta";

	return extension;
}

serial_impl_handle metacall_serial_impl_initialize()
{
	serial_impl_handle handle = malloc(sizeof(serial_impl_handle));

	return handle;
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
	size_t length, buffer_size;

	char * buffer;

	if (handle == NULL || v == NULL || size == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Serialization called with wrong arguments in MetaCall Native Format implementation");

		return NULL;
	}

	metacall_serial_impl_serialize_value(v, NULL, 0, &length);

	if (length == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Serialization invalid length calculation in MetaCall Native Format implementation");

		return NULL;
	}

	buffer_size = length + 1;

	buffer = malloc(sizeof(char) * (buffer_size));

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
		
		free(buffer);

		*size = 0;

		return NULL;
	}

	*size = buffer_size;

	return buffer;
}

/*
value metacall_serial_impl_deserialize_value(const rapidjson::Value * v)
{


	log_write("metacall", LOG_LEVEL_ERROR, "Unsuported value type in MetaCall Native Format implementation");

	return NULL;
}
*/

value metacall_serial_impl_deserialize(serial_impl_handle handle, const char * buffer, size_t size)
{
	if (handle == NULL || buffer == NULL || size == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Deserialization called with wrong arguments in MetaCall Native Format implementation");

		return NULL;
	}

	/*
	return metacall_serial_impl_deserialize_value(document);
	*/
	
	return NULL;
}

int metacall_serial_impl_destroy(serial_impl_handle handle)
{
	free(handle);

	return 0;
}
