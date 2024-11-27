/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <dynlink/dynlink.h>

#include <dynlink/dynlink_impl.h>

#include <log/log.h>

#include <string.h>

#include <mach-o/dyld.h>

#include <unistd.h>

/* -- Methods -- */

const char *dynlink_impl_interface_extension_macos(void)
{
	static const char extension_macos[] = "dylib";

	return extension_macos;
}

void dynlink_impl_interface_get_name_macos(dynlink_name name, dynlink_name_impl name_impl, size_t size)
{
	strncpy(name_impl, name, size);

	strncat(name_impl, ".", size - 1);

	strncat(name_impl, dynlink_impl_extension(), size - 1);
}

dynlink_impl dynlink_impl_interface_load_macos(dynlink handle)
{
	dynlink_flags flags = dynlink_get_flags(handle);

	unsigned long flags_impl;

	NSObjectFileImage image;

	NSModule impl;

	const char *name = dynlink_get_name_impl(handle);

	NSObjectFileImageReturnCode ret = NSCreateObjectFileImageFromFile(name, &image);

	if (ret != NSObjectFileImageSuccess)
	{
		char *error;

		switch (ret)
		{
			case NSObjectFileImageAccess:
				if (access(name, F_OK) == 0)
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

		const char *file, *error;

		NSLinkEditError(&link_edit_errors, &number, &file, &error);

		log_write("metacall", LOG_LEVEL_ERROR, "DynLink error: %s (%d) %s", name, number, error);

		return NULL;
	}

	return (dynlink_impl)impl;
}

int dynlink_impl_interface_symbol_macos(dynlink handle, dynlink_impl impl, dynlink_symbol_name name, dynlink_symbol_addr *addr)
{
	NSSymbol symbol = NSLookupSymbolInModule(impl, name);
	void *symbol_addr = NSAddressOfSymbol(symbol);

	(void)handle;

	dynlink_symbol_cast(void *, symbol_addr, *addr);

	return (*addr == NULL);
}

int dynlink_impl_interface_unload_macos(dynlink handle, dynlink_impl impl)
{
	(void)handle;

	return NSUnLinkModule(impl, 0) == TRUE ? 0 : 1;
}

dynlink_impl_interface dynlink_impl_interface_singleton(void)
{
	static struct dynlink_impl_interface_type impl_interface_macos = {
		&dynlink_impl_interface_extension_macos,
		&dynlink_impl_interface_get_name_macos,
		&dynlink_impl_interface_load_macos,
		&dynlink_impl_interface_symbol_macos,
		&dynlink_impl_interface_unload_macos,
	};

	return &impl_interface_macos;
}
