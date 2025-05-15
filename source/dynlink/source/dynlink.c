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

#include <metacall/metacall_version.h>

#include <dynlink/dynlink.h>
#include <dynlink/dynlink_impl.h>

#include <portability/portability_executable_path.h>
#include <portability/portability_path.h>

#include <stdlib.h>
#include <string.h>

/* -- Member data -- */

struct dynlink_type
{
	dynlink_path name;	 /**< Dynamically linked shared object name */
	dynlink_path path;	 /**< Dynamically linked shared object file name */
	dynlink_flags flags; /**< Dynamically linked shared object flags */
	dynlink_impl impl;	 /**< Dynamically linked shared object loader implementation */
};

/* -- Methods -- */

const char *dynlink_prefix(void)
{
	return dynlink_impl_prefix();
}

const char *dynlink_extension(void)
{
	return dynlink_impl_extension();
}

dynlink dynlink_load(const char *path, const char *name, dynlink_flags flags)
{
	if (name != NULL)
	{
		dynlink handle = malloc(sizeof(struct dynlink_type));

		if (handle != NULL)
		{
			dynlink_path name_impl;

			strncpy(handle->name, name, PORTABILITY_PATH_SIZE - 1);

			dynlink_impl_get_name(handle->name, name_impl, PORTABILITY_PATH_SIZE);

			if (path != NULL)
			{
				dynlink_path join_path;

				size_t join_path_size = portability_path_join(path, strnlen(path, PORTABILITY_PATH_SIZE) + 1, name_impl, strnlen(name_impl, PORTABILITY_PATH_SIZE) + 1, join_path, PORTABILITY_PATH_SIZE);

				(void)portability_path_canonical(join_path, join_path_size, handle->path, PORTABILITY_PATH_SIZE);
			}
			else
			{
				strncpy(handle->path, name_impl, strnlen(name_impl, PORTABILITY_PATH_SIZE) + 1);
			}

			DYNLINK_FLAGS_SET(handle->flags, flags);

			handle->impl = dynlink_impl_load(handle);

			if (handle->impl != NULL)
			{
				return handle;
			}

			free(handle);
		}
	}

	return NULL;
}

dynlink dynlink_load_absolute(const char *path, dynlink_flags flags)
{
	dynlink handle = malloc(sizeof(struct dynlink_type));
	size_t path_size, name_size, prefix_length;
	const char *prefix = dynlink_prefix();

	if (handle == NULL)
	{
		return NULL;
	}

	path_size = strnlen(path, PORTABILITY_PATH_SIZE) + 1;

	strncpy(handle->path, path, path_size);

	/* Get the library name without any extension */
	name_size = portability_path_get_name_canonical(path, path_size, handle->name, PORTABILITY_PATH_SIZE);

	/* Remove the library prefix */
	prefix_length = strlen(prefix);

	if (strncmp(prefix, handle->name, prefix_length) == 0)
	{
		size_t current, next = prefix_length, end = name_size - prefix_length;

		for (current = 0; current < end; ++current, ++next)
		{
			handle->name[current] = handle->name[next];
		}
	}

	DYNLINK_FLAGS_SET(handle->flags, flags);

	handle->impl = dynlink_impl_load(handle);

	if (handle->impl == NULL)
	{
		free(handle);
		return NULL;
	}

	return handle;
}

dynlink dynlink_load_self(dynlink_flags flags)
{
	portability_executable_path_length path_length;
	dynlink handle = malloc(sizeof(struct dynlink_type));

	if (handle == NULL)
	{
		return NULL;
	}

	/* Retrieve the executable path for the full name */
	portability_executable_path(handle->path, &path_length);

	/* Get the name without the extension */
	portability_path_get_name(handle->path, path_length + 1, handle->name, PORTABILITY_PATH_SIZE);

	/* Set the flags with the additional special flag for itself,
	this will help to identify that the handle loaded is the current executable
	and behave accordingly depending on the implementation
	*/
	DYNLINK_FLAGS_SET(handle->flags, flags);
	DYNLINK_FLAGS_ADD(handle->flags, DYNLINK_FLAGS_BIND_SELF);

	handle->impl = dynlink_impl_load(handle);

	if (handle->impl == NULL)
	{
		free(handle);
		return NULL;
	}

	return handle;
}

const char *dynlink_get_name(dynlink handle)
{
	if (handle != NULL)
	{
		return handle->name;
	}

	return NULL;
}

const char *dynlink_get_path(dynlink handle)
{
	if (handle != NULL)
	{
		return handle->path;
	}

	return NULL;
}

dynlink_flags dynlink_get_flags(dynlink handle)
{
	if (handle != NULL)
	{
		return handle->flags;
	}

	return 0;
}

dynlink_impl dynlink_get_impl(dynlink handle)
{
	if (handle != NULL)
	{
		return handle->impl;
	}

	return NULL;
}

int dynlink_symbol(dynlink handle, const char *symbol_name, dynlink_symbol_addr *symbol_address)
{
	if (handle != NULL && handle->impl != NULL && symbol_name != NULL && symbol_address != NULL)
	{
		return dynlink_impl_symbol(handle, handle->impl, symbol_name, symbol_address);
	}

	return 1;
}

void dynlink_unload(dynlink handle)
{
	if (handle != NULL)
	{
		dynlink_impl_unload(handle, handle->impl);

		free(handle);
	}
}

int dynlink_library_path(const char *name, dynlink_path path, size_t *length)
{
	dynlink_path name_impl;

	dynlink_impl_get_name(name, name_impl, PORTABILITY_PATH_SIZE);

	if (portability_library_path_find(name_impl, path, length) != 0)
	{
		return 1;
	}

	if (length != NULL)
	{
		*length = portability_path_get_directory_inplace(path, (*length) + 1) - 1;
	}
	else
	{
		(void)portability_path_get_directory_inplace(path, strnlen(path, PORTABILITY_PATH_SIZE));
	}

	return 0;
}

void dynlink_platform_name(const char *name, dynlink_path result)
{
	dynlink_impl_get_name(name, result, PORTABILITY_PATH_SIZE);
}

const char *dynlink_print_info(void)
{
	static const char dynlink_info[] =
		"Dynamic Link Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef DYNLINK_STATIC_DEFINE
		"Compiled as static library type"
#else
		"Compiled as shared library type"
#endif

		"\n";

	return dynlink_info;
}
