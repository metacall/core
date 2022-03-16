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

#include <dynlink/dynlink_impl.h>

#include <portability/portability_path.h>
#include <stdlib.h>
#include <string.h>

/* -- Methods -- */

const char *dynlink_impl_extension(void)
{
	dynlink_impl_interface_singleton singleton = dynlink_interface();

	return singleton()->extension();
}

void dynlink_impl_get_name(dynlink handle, dynlink_name_impl name_impl, size_t size)
{
	if (name_impl != NULL && size > 1)
	{
		dynlink_impl_interface_singleton singleton = dynlink_interface();

		singleton()->get_name(handle, name_impl, size);
	}
}

dynlink_impl dynlink_impl_load(dynlink handle)
{
	dynlink_impl_interface_singleton singleton = dynlink_interface();

	return singleton()->load(handle);
}

int dynlink_impl_symbol(dynlink handle, dynlink_impl impl, dynlink_symbol_name symbol_name, dynlink_symbol_addr *symbol_address)
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

static int dynlink_impl_lib_path_ends_with(dynlink_path path, dynlink_name name)
{
	if (path == NULL || name == NULL)
	{
		return 1;
	}

	size_t path_length = strlen(path);
	size_t name_length = strlen(name);

	return !(name_length <= path_length && strncmp(path + path_length - name_length, name, name_length) == 0);
}

char *dynlink_impl_lib_path(dynlink_name name)
{
	if (name != NULL)
	{
		dynlink_impl_interface_singleton singleton = dynlink_interface();

		return singleton()->lib_path(name, &dynlink_impl_lib_path_ends_with);
	}

	return NULL;
}

char *dynlink_impl_lib_dir_path(char *metacall_lib_path)
{
	/* TODO: Review this */
	size_t metacall_lib_path_size = strlen(metacall_lib_path);
	char *metacall_lib_dir_path = malloc(sizeof(char) * (metacall_lib_path_size + 1));
	size_t metacall_lib_dir_path_size = portability_path_get_directory(metacall_lib_path, metacall_lib_path_size, metacall_lib_dir_path, metacall_lib_path_size);
	metacall_lib_dir_path[metacall_lib_dir_path_size - 2] = '\0';
	return metacall_lib_dir_path;
}
