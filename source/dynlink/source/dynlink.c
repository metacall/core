/*
 *	Dynamic Link Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

/* -- Headers -- */

#include <metacall/metacall-version.h>

#include <dynlink/dynlink.h>
#include <dynlink/dynlink_impl.h>

#include <stdlib.h>
#include <string.h>

/* -- Member data -- */

typedef struct dynlink_type
{
	dynlink_name_impl				name;					/**< Dynamically linked shared object name */
	dynlink_name_impl				name_impl;				/**< Dynamically linked shared object file name */
	dynlink_flags					flags;					/**< Dynamically linked shared object flags */
	dynlink_impl					impl;					/**< Dynamically linked shared object loader implementation */

} * dynlink;

/* -- Methods -- */

const char * dynlink_extension()
{
	return dynlink_impl_extension();
}

dynlink dynlink_load(dynlink_path path, dynlink_name name, dynlink_flags flags)
{
	if (name != NULL)
	{
		dynlink handle = malloc(sizeof(struct dynlink_type));

		if (handle != NULL)
		{
			strncpy(handle->name, name, DYNLINK_NAME_IMPL_SIZE);

			dynlink_impl_get_name(handle, handle->name_impl, DYNLINK_NAME_IMPL_SIZE);

			if (path != NULL)
			{
				size_t path_length = strlen(path);

				memmove(handle->name_impl + path_length, handle->name_impl, strlen(handle->name_impl) + 1);

				memcpy(handle->name_impl, path, path_length);
			}

			handle->flags = flags;

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

dynlink_name dynlink_get_name(dynlink handle)
{
	if (handle != NULL)
	{
		return handle->name;
	}

	return NULL;
}

dynlink_name dynlink_get_name_impl(dynlink handle)
{
	if (handle != NULL)
	{
		return handle->name_impl;
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

int dynlink_symbol(dynlink handle, dynlink_symbol_name symbol_name, dynlink_symbol_addr * symbol_address)
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

const char * dynlink_print_info()
{
	static const char dynlink_info[] =
		"Dynamic Link Library " METACALL_VERSION "\n"
		"Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef DYNLINK_STATIC_DEFINE
			"Compiled as static library type"
		#else
			"Compiled as shared library type"
		#endif

		"\n";

	return dynlink_info;
}
