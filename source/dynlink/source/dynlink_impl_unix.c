/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
	#define __USE_GNU
#endif

#include <dlfcn.h>

/* -- Methods -- */

const char *dynlink_impl_interface_extension_unix(void)
{
	static const char extension_unix[] = "so";

	return extension_unix;
}

void dynlink_impl_interface_get_name_unix(dynlink_name name, dynlink_name_impl name_impl, size_t size)
{
	strncpy(name_impl, "lib", size);

	strncat(name_impl, name, size - 1);

	strncat(name_impl, ".", size - 1);

	strncat(name_impl, dynlink_impl_extension(), size - 1);
}

dynlink_impl dynlink_impl_interface_load_unix(dynlink handle)
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

	impl = dlopen(dynlink_get_name_impl(handle), flags_impl);

	if (impl != NULL)
	{
		return (dynlink_impl)impl;
	}

	log_write("metacall", LOG_LEVEL_ERROR, "DynLink error: %s", dlerror());

	return NULL;
}

int dynlink_impl_interface_symbol_unix(dynlink handle, dynlink_impl impl, dynlink_symbol_name name, dynlink_symbol_addr *addr)
{
	void *symbol = dlsym(impl, name);

	(void)handle;

	*addr = (dynlink_symbol_addr)symbol;

	return (*addr == NULL);
}

int dynlink_impl_interface_unload_unix(dynlink handle, dynlink_impl impl)
{
	(void)handle;

#if defined(__ADDRESS_SANITIZER__) || defined(__THREAD_SANITIZER__)
	/* Disable dlclose when running with address sanitizer in order to maintain stacktraces */
	(void)impl;
	return 0;
#else
	return dlclose(impl);
#endif
}

dynlink_impl_interface dynlink_impl_interface_singleton_unix(void)
{
	static struct dynlink_impl_interface_type impl_interface_unix = {
		&dynlink_impl_interface_extension_unix,
		&dynlink_impl_interface_get_name_unix,
		&dynlink_impl_interface_load_unix,
		&dynlink_impl_interface_symbol_unix,
		&dynlink_impl_interface_unload_unix,
	};

	return &impl_interface_unix;
}
