/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <serial/serial_impl.h>
#include <serial/serial_interface.h>

#include <dynlink/dynlink.h>

#include <log/log.h>

#include <string.h>

/* -- Definitions -- */

#define SERIAL_DYNLINK_NAME_SIZE 0x40
#define SERIAL_DYNLINK_SUFFIX	 "_serial"

/* -- Member Data -- */

struct serial_impl_type
{
	dynlink handle;
	serial_interface iface;
};

/* -- Private Methods -- */

static dynlink serial_impl_load_dynlink(const char *path, const char *name);

static int serial_impl_load_symbol(dynlink handle, const char *name, dynlink_symbol_addr *singleton_addr_ptr);

/* -- Methods -- */

dynlink serial_impl_load_dynlink(const char *path, const char *name)
{
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	const char serial_dynlink_suffix[] = SERIAL_DYNLINK_SUFFIX "d";
#else
	const char serial_dynlink_suffix[] = SERIAL_DYNLINK_SUFFIX;
#endif

#define SERIAL_DYNLINK_NAME_FULL_SIZE \
	(sizeof(serial_dynlink_suffix) + SERIAL_DYNLINK_NAME_SIZE)

	char serial_dynlink_name[SERIAL_DYNLINK_NAME_FULL_SIZE];

	strncpy(serial_dynlink_name, name, SERIAL_DYNLINK_NAME_FULL_SIZE - 1);

	strncat(serial_dynlink_name, serial_dynlink_suffix,
		SERIAL_DYNLINK_NAME_FULL_SIZE - strnlen(serial_dynlink_name, SERIAL_DYNLINK_NAME_FULL_SIZE - 1) - 1);

#undef SERIAL_DYNLINK_NAME_FULL_SIZE

	log_write("metacall", LOG_LEVEL_DEBUG, "Loading serial plugin: %s", serial_dynlink_name);

	return dynlink_load(path, serial_dynlink_name, DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);
}

int serial_impl_load_symbol(dynlink handle, const char *name, dynlink_symbol_addr *singleton_addr_ptr)
{
	const char serial_dynlink_symbol_prefix[] = DYNLINK_SYMBOL_STR("");
	const char serial_dynlink_symbol_suffix[] = "_serial_impl_interface_singleton";

#define SERIAL_DYNLINK_SYMBOL_SIZE \
	(sizeof(serial_dynlink_symbol_prefix) + SERIAL_DYNLINK_NAME_SIZE + sizeof(serial_dynlink_symbol_suffix))

	char serial_dynlink_symbol[SERIAL_DYNLINK_SYMBOL_SIZE];

	strncpy(serial_dynlink_symbol, serial_dynlink_symbol_prefix, SERIAL_DYNLINK_SYMBOL_SIZE);

	strncat(serial_dynlink_symbol, name,
		SERIAL_DYNLINK_SYMBOL_SIZE - strnlen(serial_dynlink_symbol, SERIAL_DYNLINK_SYMBOL_SIZE - 1) - 1);

	strncat(serial_dynlink_symbol, serial_dynlink_symbol_suffix,
		SERIAL_DYNLINK_SYMBOL_SIZE - strnlen(serial_dynlink_symbol, SERIAL_DYNLINK_SYMBOL_SIZE - 1) - 1);

#undef SERIAL_DYNLINK_SYMBOL_SIZE

	log_write("metacall", LOG_LEVEL_DEBUG, "Loading serial symbol: %s", serial_dynlink_symbol);

	return dynlink_symbol(handle, serial_dynlink_symbol, singleton_addr_ptr);
}

serial_impl serial_impl_create()
{
	serial_impl impl = malloc(sizeof(struct serial_impl_type));

	if (impl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation allocation");

		return NULL;
	}

	impl->handle = NULL;
	impl->iface = NULL;

	return impl;
}

const char *serial_impl_extension(serial_impl impl)
{
	return impl->iface->extension();
}

int serial_impl_load(serial_impl impl, const char *path, const char *name)
{
	dynlink_symbol_addr singleton_addr;

	serial_interface_singleton iface_singleton;

	impl->handle = serial_impl_load_dynlink(path, name);

	if (impl->handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial dynlink loading");

		return 1;
	}

	if (serial_impl_load_symbol(impl->handle, name, &singleton_addr) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial dynlink symbol loading");

		dynlink_unload(impl->handle);

		impl->handle = NULL;

		return 1;
	}

	iface_singleton = (serial_interface_singleton)DYNLINK_SYMBOL_GET(singleton_addr);

	if (iface_singleton == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial interface singleton access");

		dynlink_unload(impl->handle);

		impl->handle = NULL;

		return 1;
	}

	impl->iface = iface_singleton();

	return 0;
}

char *serial_impl_serialize(serial_impl impl, value v, size_t *size, memory_allocator allocator)
{
	serial_impl_handle handle = impl->iface->initialize(allocator);

	char *buffer;

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation handle initialization");

		return NULL;
	}

	buffer = impl->iface->serialize(handle, v, size);

	if (buffer == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation handle serialization");
	}

	if (impl->iface->destroy(handle) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation handle destruction");
	}

	return buffer;
}

value serial_impl_deserialize(serial_impl impl, const char *buffer, size_t size, memory_allocator allocator)
{
	serial_impl_handle handle = impl->iface->initialize(allocator);

	value v;

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation handle initialization");

		return NULL;
	}

	v = impl->iface->deserialize(handle, buffer, size);

	if (v == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation handle deserialization");
	}

	if (impl->iface->destroy(handle) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation handle destruction");
	}

	return v;
}

int serial_impl_unload(serial_impl impl)
{
	if (impl != NULL)
	{
		impl->iface = NULL;

		if (impl->handle != NULL)
		{
			dynlink_unload(impl->handle);

			impl->handle = NULL;
		}
	}

	return 0;
}

int serial_impl_destroy(serial_impl impl)
{
	if (impl != NULL)
	{
		if (impl->handle != NULL)
		{
			dynlink_unload(impl->handle);
		}

		free(impl);
	}

	return 0;
}
