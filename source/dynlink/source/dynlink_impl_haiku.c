/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#if defined(__HAIKU__)

/* -- Headers -- */

#include <dynlink/dynlink.h>

#include <dynlink/dynlink_impl.h>

#include <log/log.h>

#include <string.h>

#include <stdint.h>

#include <dlfcn.h>

#include <be/kernel/image.h>

/* -- Methods -- */

const char *dynlink_impl_interface_prefix_haiku(void)
{
	static const char prefix_haiku[] = "lib";

	return prefix_haiku;
}

const char *dynlink_impl_interface_extension_haiku(void)
{
	static const char extension_haiku[] = "so";

	return extension_haiku;
}

dynlink_impl dynlink_impl_interface_load_haiku(dynlink handle)
{
	dynlink_flags flags = dynlink_get_flags(handle);
	image_id impl = 0;

	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_SELF))
	{
		int mode;
		void *self_impl;

		DYNLINK_FLAGS_SET(mode, 0);

		if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_NOW))
		{
			DYNLINK_FLAGS_ADD(mode, RTLD_NOW);
		}

		if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_LAZY))
		{
			DYNLINK_FLAGS_ADD(mode, RTLD_LAZY);
		}

		if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_LOCAL))
		{
			DYNLINK_FLAGS_ADD(mode, RTLD_LOCAL);
		}

		if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_GLOBAL))
		{
			DYNLINK_FLAGS_ADD(mode, RTLD_GLOBAL);
		}

		self_impl = dlopen(NULL, mode);

		if (self_impl == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "DynLink error: %s", dlerror());
			return NULL;
		}

		return self_impl;
	}

	impl = load_add_on(dynlink_get_path(handle));

	if (impl < B_NO_ERROR)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "DynLink error: failed to load BeOS/Haiku image add-on with error code %d", (int)impl);
		return NULL;
	}

	return (dynlink_impl)(intptr_t)impl;
}

int dynlink_impl_interface_symbol_haiku(dynlink handle, dynlink_impl impl, const char *name, dynlink_symbol_addr *addr)
{
	void *symbol = NULL;

	if (DYNLINK_FLAGS_CHECK(dynlink_get_flags(handle), DYNLINK_FLAGS_BIND_SELF))
	{
		symbol = dlsym(impl, name);
	}
	else
	{
		int err = get_image_symbol((image_id)(intptr_t)impl, name, B_SYMBOL_TYPE_ANY, &symbol);

		if (err != B_OK)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "DynLink error: failed to load BeOS/Haiku symbol %s", name);
			return 1;
		}
	}

	dynlink_symbol_cast(void *, symbol, *addr);

	return (*addr == NULL);
}

int dynlink_impl_interface_unload_haiku(dynlink handle, dynlink_impl impl)
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
	// return ((image_id)(intptr_t)impl > 0) && (unload_add_on((image_id)(intptr_t)impl) < B_NO_ERROR);
	(void)impl;
	return 0;
#endif
}

dynlink_impl_interface dynlink_impl_interface_singleton(void)
{
	static struct dynlink_impl_interface_type impl_interface_haiku = {
		&dynlink_impl_interface_prefix_haiku,
		&dynlink_impl_interface_extension_haiku,
		&dynlink_impl_interface_load_haiku,
		&dynlink_impl_interface_symbol_haiku,
		&dynlink_impl_interface_unload_haiku,
	};

	return &impl_interface_haiku;
}

#elif defined(__BEOS__)
#include "dynlink_impl_beos.c"
#endif
