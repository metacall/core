/*
 *	Dynamic Link Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

/* -- Headers -- */

#include <dynlink/dynlink_impl.h>

#include <stdlib.h>

/* -- Methods -- */

const char * dynlink_impl_extension(void)
{
	dynlink_impl_interface_singleton singleton = dynlink_interface();

	return singleton()->extension();
}

void dynlink_impl_get_name(dynlink handle, dynlink_name_impl name_impl, size_t length)
{
	if (name_impl != NULL && length > 0)
	{
		dynlink_impl_interface_singleton singleton = dynlink_interface();

		singleton()->get_name(handle, name_impl, length);
	}
}

dynlink_impl dynlink_impl_load(dynlink handle)
{
	dynlink_impl_interface_singleton singleton = dynlink_interface();

	return singleton()->load(handle);
}

int dynlink_impl_symbol(dynlink handle, dynlink_impl impl, dynlink_symbol_name symbol_name, dynlink_symbol_addr * symbol_address)
{
	if (impl != NULL)
	{
		dynlink_impl_interface_singleton singleton = dynlink_interface();

		return singleton()->symbol(handle, impl, symbol_name, symbol_address);
	}

	return 1;
}

void dynlink_impl_unload(dynlink handle, dynlink_impl impl)
{
	if (impl != NULL)
	{
		dynlink_impl_interface_singleton singleton = dynlink_interface();

		singleton()->unload(handle, impl);
	}
}
