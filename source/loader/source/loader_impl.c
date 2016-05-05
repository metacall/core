/************************************************************************/
/*	Loader Library by Parra Studios										*/
/*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																		*/
/*	A library for loading executable code at run-time into a process.	*/
/*																		*/
/************************************************************************/

#include <loader/loader_impl.h>

#include <adt/hash_map.h>
#include <adt/hash_map_str.h>

#include <stdlib.h>
#include <string.h>

typedef struct loader_handle_impl_type
{
	loader_naming_name name;
	loader_handle handle;
	context ctx;

} * loader_handle_impl;

typedef struct loader_impl_type
{
	loader_naming_extension extension;
	loader_impl_interface_singleton singleton;
	hash_map handle_impl_map;

} * loader_impl;

loader_impl loader_impl_create(loader_naming_extension extension)
{
	if (extension != NULL)
	{
		loader_impl impl = malloc(sizeof(struct loader_impl_type));

		if (impl != NULL)
		{
			/* load library ? */

			/* load singleton */

			/* create callback ... */

			{
				impl->handle_impl_map = hash_map_create(&hash_map_cb_hash_str, &hash_map_cb_compare_str);

				strncpy(impl->extension, extension, LOADER_NAMING_EXTENSION_SIZE);
			}

			return impl;
		}
	}

	return NULL;
}

loader_impl_interface loader_impl_symbol(loader_impl impl)
{
	if (impl != NULL && impl->singleton != NULL)
	{
		return impl->singleton();
	}

	return NULL;
}

loader_naming_extension * loader_impl_extension(loader_impl impl)
{
	if (impl != NULL)
	{
		return &impl->extension;
	}

	return NULL;
}

int loader_impl_load(loader_impl impl, loader_naming_name name)
{
	if (impl != NULL)
	{
		loader_impl_interface impl_interface = loader_impl_symbol(impl);

		loader_handle handle = impl_interface->load(impl, name);

		if (handle != NULL)
		{
			/* insert into handle_impl_map */

			/* discovery ? */

			return 0;
		}
	}

	return 1;
}

void loader_impl_destroy(loader_impl impl)
{
	if (impl != NULL)
	{
		/* destroy callback ... */

		/* impl->loader_handle_map */

		free(impl);
	}
}
