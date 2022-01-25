/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#define _GNU_SOURCE

#include <dynlink/dynlink.h>

#include <dynlink/dynlink_impl.h>

#include <log/log.h>

#include <string.h>

#include <dlfcn.h>

#include <link.h>

#include <stdio.h>

#define METACALL_LIB_NAME "libmetacall"

/* -- Methods -- */

const char *dynlink_impl_interface_extension_unix(void)
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

#if defined(__ADDRESS_SANITIZER__)
	/* Disable dlclose when running with address sanitizer in order to maintain stacktraces */
	(void)impl;
	return 0;
#else
	return dlclose(impl);
#endif
}

struct metacall_lib_data
{
	char *metacall_lib_name;
	char *metacall_lib_path;
};

static int str_ends_with(const char *s, const char *suffix)
{
	size_t slen = strlen(s);
	size_t suffix_len = strlen(suffix);
	return suffix_len <= slen && !strcmp(s + slen - suffix_len, suffix);
}

static char *metacall_custom_lib_path(char *metacall_lib_path, char *metacall_lib_name)
{
	size_t lib_path_len = strlen(metacall_lib_path) - (strlen(metacall_lib_name) + 5);
	char *custom_lib_path = malloc(sizeof(char) * (lib_path_len + 1));
	custom_lib_path[lib_path_len] = 0;
	strncpy(custom_lib_path, metacall_lib_path, lib_path_len);
	return custom_lib_path;
}

static int phdr_callback(struct dl_phdr_info *i, size_t size, void *data)
{
	char *lib_path = i->dlpi_name;
	struct metacall_lib_data *metacall_data_inst = (struct metacall_data *)data;
	if (str_ends_with(lib_path, metacall_data_inst->metacall_lib_name))
	{
		metacall_data_inst->metacall_lib_path = metacall_custom_lib_path(lib_path, metacall_data_inst->metacall_lib_name);
		return 1;
	}
	return 0;
}

// TODO: free returned string pointer
const char *dynlink_impl_interface_get_metacall_lib_path_unix(void)
{
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	static char metacall_lib_name[17] = METACALL_LIB_NAME "d.so";
#else
	static char metacall_lib_name[16] = METACALL_LIB_NAME ".so";
#endif

	char *metacall_lib_path;
	struct metacall_lib_data metacall_data_inst = {
		.metacall_lib_name = metacall_lib_name,
		.metacall_lib_path = NULL
	};
	dl_iterate_phdr(phdr_callback, (void *)&metacall_data_inst);
	return metacall_data_inst.metacall_lib_path;
}

dynlink_impl_interface dynlink_impl_interface_singleton_unix(void)
{
	static struct dynlink_impl_interface_type impl_interface_unix = {
		&dynlink_impl_interface_extension_unix,
		&dynlink_impl_interface_get_name_unix,
		&dynlink_impl_interface_load_unix,
		&dynlink_impl_interface_symbol_unix,
		&dynlink_impl_interface_unload_unix,
		&dynlink_impl_interface_get_metacall_lib_path_unix,
	};

	return &impl_interface_unix;
}
