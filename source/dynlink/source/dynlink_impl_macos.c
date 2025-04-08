/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

/* -- Member Data -- */

static void *dynlink_impl_global_handle_macos = NULL;

/* -- Methods -- */

const char *dynlink_impl_interface_prefix_macos(void)
{
	static const char prefix_macos[] = "lib";

	return prefix_macos;
}

const char *dynlink_impl_interface_extension_macos(void)
{
	static const char extension_macos[] = "dylib";

	return extension_macos;
}

dynlink_impl dynlink_impl_interface_load_macos(dynlink handle)
{
	dynlink_flags flags = dynlink_get_flags(handle);
	NSModule impl;

	if (!DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_SELF))
	{
		unsigned long flags_impl;
		NSObjectFileImage image;
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
	}
	else
	{
		/* We return this for identifying the global handle when loading symbols of the current process */
		impl = (void *)(&dynlink_impl_global_handle_macos);
	}

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
	dynlink_flags flags = dynlink_get_flags(handle);
	NSSymbol symbol;
	void *symbol_addr;

	(void)handle;

	/* Skip unlink when using global handle for loading symbols of the current process */
	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_SELF))
	{
		/* Global context, use NSLookupAndBindSymbol */
		if (!NSIsSymbolNameDefined(name))
		{
			return 1;
		}

		symbol = NSLookupAndBindSymbol(name);
	}
	else
	{
		symbol = NSLookupSymbolInModule(impl, name);
	}

	symbol_addr = NSAddressOfSymbol(symbol);

	dynlink_symbol_cast(void *, symbol_addr, *addr);

	return (*addr == NULL);
}

int dynlink_impl_interface_unload_macos(dynlink handle, dynlink_impl impl)
{
	dynlink_flags flags = dynlink_get_flags(handle);

	(void)handle;

	/* Skip unlink when using global handle for loading symbols of the current process */
	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_SELF))
	{
		return 0;
	}

#if defined(__MEMORYCHECK__) || defined(__ADDRESS_SANITIZER__) || defined(__THREAD_SANITIZER__) || defined(__MEMORY_SANITIZER__)
	/* Disable dlclose when running with address sanitizer in order to maintain stacktraces */
	(void)impl;
	return 0;
#else
	return NSUnLinkModule(impl, 0) == TRUE ? 0 : 1;
#endif
}

dynlink_impl_interface dynlink_impl_interface_singleton(void)
{
	static struct dynlink_impl_interface_type impl_interface_macos = {
		&dynlink_impl_interface_prefix_macos,
		&dynlink_impl_interface_extension_macos,
		&dynlink_impl_interface_load_macos,
		&dynlink_impl_interface_symbol_macos,
		&dynlink_impl_interface_unload_macos,
	};

	return &impl_interface_macos;
}
