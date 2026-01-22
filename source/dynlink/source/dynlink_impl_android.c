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



#include <dynlink/dynlink.h>

#include <dynlink/dynlink_impl.h>

#include <log/log.h>

#include <string.h>

#include <dlfcn.h>


const char *dynlink_impl_interface_prefix_android(void)
{
	
	static const char prefix_android[] = "lib";

	return prefix_android;
}

const char *dynlink_impl_interface_extension_android(void)
{
	
	static const char extension_android[] = "so";

	return extension_android;
}

dynlink_impl dynlink_impl_interface_load_android(dynlink handle)
{
	dynlink_flags flags = dynlink_get_flags(handle);
	int flags_impl;
	void *impl;

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

	
	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_SELF))
	{
		
		impl = dlopen(NULL, flags_impl);
	}
	else
	{
		
		impl = dlopen(dynlink_get_path(handle), flags_impl);
	}

	if (impl == NULL)
	{
		const char *error_msg = dlerror();

		log_write("metacall", LOG_LEVEL_ERROR, "DynLink Android error: %s",
			error_msg ? error_msg : "Unknown error");

		return NULL;
	}

	return (dynlink_impl)impl;
}

int dynlink_impl_interface_symbol_android(dynlink handle, dynlink_impl impl, const char *name, dynlink_symbol_addr *addr)
{
	void *symbol;

	(void)handle;

	
	dlerror();

	symbol = dlsym(impl, name);

	
	if (symbol == NULL)
	{
		const char *error_msg = dlerror();

		if (error_msg != NULL)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "DynLink Android: Symbol '%s' not found: %s", name, error_msg);
			*addr = NULL;
			return 1;
		}
	}

	dynlink_symbol_cast(void *, symbol, *addr);

	return 0;
}

int dynlink_impl_interface_unload_android(dynlink handle, dynlink_impl impl)
{
	dynlink_flags flags = dynlink_get_flags(handle);

	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_SELF))
	{
		return 0;
	}

#if defined(__MEMORYCHECK__) || defined(__ADDRESS_SANITIZER__) || defined(__THREAD_SANITIZER__) || defined(__MEMORY_SANITIZER__)
	(void)impl;
	return 0;
#else
	return dlclose(impl);
#endif
}

dynlink_impl_interface dynlink_impl_interface_singleton(void)
{
	static struct dynlink_impl_interface_type impl_interface_android = {
		&dynlink_impl_interface_prefix_android,
		&dynlink_impl_interface_extension_android,
		&dynlink_impl_interface_load_android,
		&dynlink_impl_interface_symbol_android,
		&dynlink_impl_interface_unload_android,
	};

	return &impl_interface_android;
}
