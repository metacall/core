/*
 *	Dynamic Link Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

/* -- Headers -- */

#include <dynlink/dynlink.h>

#include <dynlink/dynlink_impl.h>

#include <string.h>

#include <dlfcn.h>

/* -- Methods -- */

const char * dynlink_impl_interface_extension_unix(void)
{
	static const char extension_unix[0x03] = "so";

	return extension_unix;
}

void dynlink_impl_interface_get_name_unix(dynlink handle, dynlink_name_impl name_impl, size_t length)
{
	strncpy(name_impl, "lib", length);

	strncat(name_impl, dynlink_get_name(handle), length);

	strncat(name_impl, ".", length);

	strncat(name_impl, dynlink_impl_extension(), length);
}

dynlink_impl dynlink_impl_interface_load_unix(dynlink handle)
{
	dynlink_flags flags = dynlink_get_flags(handle);

	int flags_impl;

	void * impl;

	DYNLINK_FLAGS_SET(flags_impl, 0);

	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_NOW))
	{
		DYNLINK_FLAGS_ADD(flags_impl, RTLD_NOW);
	}

	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_LAZY))
	{
		DYNLINK_FLAGS_ADD(flags_impl, RTLD_LAZY);
	}

	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_LOCAL))
	{
		DYNLINK_FLAGS_ADD(flags_impl, RTLD_LOCAL);
	}

	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_GLOBAL))
	{
		DYNLINK_FLAGS_ADD(flags_impl, RTLD_GLOBAL);
	}

	impl = dlopen(dynlink_get_name_impl(handle), flags_impl);

	if (impl != NULL)
	{
		return (dynlink_impl)impl;
	}

	return NULL;
}

int dynlink_impl_interface_symbol_unix(dynlink handle, dynlink_impl impl, dynlink_symbol_name name, dynlink_symbol_addr * addr)
{
	void * symbol = dlsym(impl, name);

	(void)handle;

	*addr = (dynlink_symbol_addr)symbol;

	return (*addr == NULL);
}

int dynlink_impl_interface_unload_unix(dynlink handle, dynlink_impl impl)
{
	(void)handle;

	return dlclose(impl);
}

dynlink_impl_interface dynlink_impl_interface_singleton_unix(void)
{
	static struct dynlink_impl_interface_type impl_interface_unix =
	{
		&dynlink_impl_interface_extension_unix,
		&dynlink_impl_interface_get_name_unix,
		&dynlink_impl_interface_load_unix,
		&dynlink_impl_interface_symbol_unix,
		&dynlink_impl_interface_unload_unix
	};

	return &impl_interface_unix;
}
