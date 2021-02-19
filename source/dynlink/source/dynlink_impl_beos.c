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

#include <log/log.h>

#include <string.h>

#include <be/kernel/image.h>

/* -- Methods -- */

const char * dynlink_impl_interface_extension_beos(void)
{
	static const char extension_beos[0x03] = "so";

	return extension_beos;
}

void dynlink_impl_interface_get_name_beos(dynlink handle, dynlink_name_impl name_impl, size_t length)
{
	strncpy(name_impl, "lib", length);

	strncat(name_impl, dynlink_get_name(handle), length);

	strncat(name_impl, ".", length);

	strncat(name_impl, dynlink_impl_extension(), length);
}

dynlink_impl dynlink_impl_interface_load_beos(dynlink handle)
{
	dynlink_flags flags = dynlink_get_flags(handle);

	int flags_impl;

	void* impl;

	DYNLINK_FLAGS_SET(flags_impl, 0);

	impl = (void*)load_add_on(dynlink_get_name_impl(handle));

	if (impl != NULL || impl == B_OK)
	{
		return (dynlink_impl)impl;
	}

	log_write("metacall", LOG_LEVEL_ERROR, "DynLink error: failed to load BeOS/Haiku image add-on");

	return NULL;
}

int dynlink_impl_interface_symbol_beos(dynlink handle, dynlink_impl impl, dynlink_symbol_name name, dynlink_symbol_addr * addr)
{
	void * symbol = NULL;
	get_image_symbol((image_id)impl, name, B_SYMBOL_TYPE_TEXT, &symbol);

	(void)handle;

	*addr = (dynlink_symbol_addr)symbol;

	return (*addr == NULL);
}

int dynlink_impl_interface_unload_beos(dynlink handle, dynlink_impl impl)
{
	(void)handle;

	#if defined(__ADDRESS_SANITIZER__)
		/* Disable dlclose when running with address sanitizer in order to maintain stacktraces */
		(void)impl;
		return 0;
	#else
		return unload_add_on((image_id)impl);
	#endif
}

dynlink_impl_interface dynlink_impl_interface_singleton_beos(void)
{
	static struct dynlink_impl_interface_type impl_interface_beos =
	{
		&dynlink_impl_interface_extension_beos,
		&dynlink_impl_interface_get_name_beos,
		&dynlink_impl_interface_load_beos,
		&dynlink_impl_interface_symbol_beos,
		&dynlink_impl_interface_unload_beos
	};

	return &impl_interface_beos;
}
