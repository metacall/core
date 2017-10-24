/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <serial/serial_impl.h>
#include <serial/serial_interface.h>

#include <dynlink/dynlink.h>

#include <string.h>

/* -- Member Data -- */

struct serial_impl_type
{
	char * path;
	serial_interface iface;
	dynlink handle;
};

/* -- Methods -- */

serial_impl serial_impl_create(const char * name, const char * path)
{

}

const char * serial_impl_extension(serial_impl impl)
{
	return impl->iface->extension();
}

int serial_impl_load(serial_impl impl)
{

}

const char * serial_impl_serialize(serial_impl impl, value v, size_t * size)
{

}

value serial_impl_deserialize(serial_impl impl, const char * buffer, size_t size)
{

}

int serial_impl_unload(serial_impl impl)
{

}

int serial_impl_destroy(serial_impl impl)
{
	if (impl != NULL)
	{
		if (impl->path != NULL)
		{
			free(impl->path);
		}

		if (impl->handle != NULL)
		{
			dynlink_unload(impl->handle);
		}

		free(impl);
	}

	return 0;
}
