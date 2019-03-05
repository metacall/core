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

#include <mach-o/dyld.h>

#include <unistd.h>

/* -- Methods -- */

const char * dynlink_impl_interface_extension_macos(void)
{
	static const char extension_macos[0x07] = "bundle";

	return extension_macos;
}

void dynlink_impl_interface_get_name_macos(dynlink handle, dynlink_name_impl name_impl, size_t length)
{
	strncpy(name_impl, dynlink_get_name(handle), length);

	strncat(name_impl, ".", length);

	strncat(name_impl, dynlink_impl_extension(), length);
}

dynlink_impl dynlink_impl_interface_load_macos(dynlink handle)
{
	dynlink_flags flags = dynlink_get_flags(handle);

	unsigned long flags_impl;

	NSObjectFileImage image;

	NSModule impl;

	const char * name = dynlink_get_name_impl(handle);

	NSObjectFileImageReturnCode ret = NSCreateObjectFileImageFromFile(name, &image);

	if (ret != NSObjectFileImageSuccess)
	{
		char * error;

		switch (ret)
		{
			case NSObjectFileImageAccess:
				if (access (name, F_OK) == 0)
				{
					error = "DynLink error: %s permission denied";
				}
				else
				{
					error = "DynLink error: %s no such file or directory";
				}
			case NSObjectFileImageArch:
				error = "DynLink error: %s is not built for the current architecture";
				break;
			case NSObjectFileImageInappropriateFile:
			case NSObjectFileImageFormat:
				error = "DynLink error: %s is not a loadable module";
				break;
			default:
				error = "DynLink error: unknown error for %s";
				break;
		}

		log_write("metacall", LOG_LEVEL_ERROR, error, name);

		return NULL;
	}

	DYNLINK_FLAGS_SET(flags_impl, NSLINKMODULE_OPTION_RETURN_ON_ERROR);

	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_LOCAL))
	{
		DYNLINK_FLAGS_ADD(flags_impl, NSLINKMODULE_OPTION_PRIVATE);
	}

	if (!DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_LAZY))
	{
		DYNLINK_FLAGS_ADD(flags_impl, NSLINKMODULE_OPTION_BINDNOW);
	}

	impl = NSLinkModule(image, name, flags_impl);

	NSDestroyObjectFileImage(image);

	if (impl == NULL)
	{
		NSLinkEditErrors link_edit_errors;

		int number;

		const char * file, * error;

		NSLinkEditError(&link_edit_errors, &number, &file, &error);

		log_write("metacall", LOG_LEVEL_ERROR, "DynLink error: %s (%d) %s", name, number, error);

		return NULL;
	}

	return (dynlink_impl)impl;
}

int dynlink_impl_interface_symbol_macos(dynlink handle, dynlink_impl impl, dynlink_symbol_name name, dynlink_symbol_addr * addr)
{
	NSSymbol symbol = NSLookupSymbolInModule(impl, name);

	(void)handle;

	*addr = (dynlink_symbol_addr)NSAddressOfSymbol(symbol);

	return (*addr == NULL);
}

int dynlink_impl_interface_unload_macos(dynlink handle, dynlink_impl impl)
{
	(void)handle;

	return NSUnLinkModule(impl, 0) == TRUE ? 0 : 1;
}

dynlink_impl_interface dynlink_impl_interface_singleton_macos(void)
{
	static struct dynlink_impl_interface_type impl_interface_macos =
	{
		&dynlink_impl_interface_extension_macos,
		&dynlink_impl_interface_get_name_macos,
		&dynlink_impl_interface_load_macos,
		&dynlink_impl_interface_symbol_macos,
		&dynlink_impl_interface_unload_macos
	};

	return &impl_interface_macos;
}
