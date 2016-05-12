/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#include <loader/loader_impl.h>

#include <reflect/context.h>

#include <adt/hash_map.h>
#include <adt/hash_map_str.h>

#include <dynlink/dynlink.h>

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
	dynlink handle;
	loader_impl_interface_singleton singleton;
	hash_map handle_impl_map;

} * loader_impl;

dynlink loader_impl_dynlink_load(loader_naming_extension extension)
{
	const char loader_dynlink_suffix[] = "_loader";

	#define LOADER_DYNLINK_NAME_SIZE (sizeof(loader_dynlink_suffix) + LOADER_NAMING_EXTENSION_SIZE)

	char loader_dynlink_name[LOADER_DYNLINK_NAME_SIZE];

	strncpy(loader_dynlink_name, extension, LOADER_NAMING_EXTENSION_SIZE);

	strncat(loader_dynlink_name, loader_dynlink_suffix, LOADER_DYNLINK_NAME_SIZE);

	#undef LOADER_DYNLINK_NAME_SIZE

	return dynlink_load(loader_dynlink_name, DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);
}

int loader_impl_dynlink_symbol(loader_impl impl, loader_naming_extension extension, dynlink_symbol_addr * singleton_addr_ptr)
{
	const char loader_dynlink_symbol_prefix[] = DYNLINK_SYMBOL_NAME_STR(loader_impl_interface_singleton_);

	#define LOADER_DYNLINK_SYMBOL_SIZE (sizeof(loader_dynlink_symbol_prefix) + LOADER_NAMING_EXTENSION_SIZE)

	char loader_dynlink_symbol[LOADER_DYNLINK_SYMBOL_SIZE];

	strncpy(loader_dynlink_symbol, loader_dynlink_symbol_prefix, sizeof(loader_dynlink_symbol_prefix));

	strncat(loader_dynlink_symbol, extension, LOADER_DYNLINK_SYMBOL_SIZE);

	#undef LOADER_DYNLINK_SYMBOL_SIZE

	return dynlink_symbol(impl->handle, loader_dynlink_symbol, singleton_addr_ptr);
}

void loader_impl_dynlink_destroy(loader_impl impl)
{
	dynlink_unload(impl->handle);
}

loader_impl loader_impl_create(loader_naming_extension extension)
{
	if (extension != NULL)
	{
		loader_impl impl = malloc(sizeof(struct loader_impl_type));

		if (impl != NULL)
		{
			impl->handle = loader_impl_dynlink_load(extension);

			if (impl->handle != NULL)
			{
				dynlink_symbol_addr singleton_addr;

				if (loader_impl_dynlink_symbol(impl, extension, &singleton_addr) == 0)
				{
					impl->singleton = (loader_impl_interface_singleton)DYNLINK_SYMBOL_GET(singleton_addr);

					impl->handle_impl_map = hash_map_create(&hash_map_cb_hash_str, &hash_map_cb_compare_str);

					strncpy(impl->extension, extension, LOADER_NAMING_EXTENSION_SIZE);

					if (impl->singleton()->initialize(impl) == 0)
					{
						return impl;
					}

					hash_map_destroy(impl->handle_impl_map);
				}
			}
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

loader_handle_impl loader_impl_load_handle(loader_handle handle, loader_naming_name name)
{
	loader_handle_impl handle_impl = malloc(sizeof(struct loader_handle_impl_type));

	if (handle_impl != NULL)
	{
		strncpy(handle_impl->name, name, LOADER_NAMING_NAME_SIZE);

		handle_impl->handle = handle;

		handle_impl->ctx = context_create(handle_impl->name);

		if (handle_impl->ctx != NULL)
		{
			return handle_impl;
		}

		free(handle_impl);
	}

	return NULL;
}

void loader_impl_destroy_handle(loader_handle_impl handle_impl)
{
	if (handle_impl != NULL)
	{
		context_destroy(handle_impl->ctx);

		free(handle_impl);
	}
}

int loader_impl_load(loader_impl impl, loader_naming_name name)
{
	if (impl != NULL)
	{
		loader_impl_interface interface_impl = loader_impl_symbol(impl);

		loader_handle handle = interface_impl->load(impl, name);

		if (handle != NULL)
		{
			loader_handle_impl handle_impl = loader_impl_load_handle(handle, name);

			if (handle_impl != NULL)
			{
				if (hash_map_insert(impl->handle_impl_map, handle_impl->name, handle_impl) == 0)
				{
					return interface_impl->discover(impl, handle_impl->handle, handle_impl->ctx);
				}

				loader_impl_destroy_handle(handle_impl);
			}

			if (interface_impl->clear(impl, handle) != 0)
			{
				return 1;
			}
		}
	}

	return 1;
}

int loader_impl_destroy_map_cb_iterate(hash_map map, hash_map_key key, hash_map_value value, hash_map_cb_iterate_args args)
{
	if (map != NULL && key != NULL && value != NULL && args == NULL)
	{
		loader_handle_impl handle_impl = value;

		loader_impl_destroy_handle(handle_impl);

		return 0;
	}

	return 1;
}

void loader_impl_destroy(loader_impl impl)
{
	if (impl != NULL)
	{
		loader_impl_interface interface_impl = loader_impl_symbol(impl);

		if (interface_impl->destroy(impl) != 0)
		{
			/* error */
		}

		hash_map_iterate(impl->handle_impl_map, &loader_impl_destroy_map_cb_iterate, NULL);

		loader_impl_dynlink_destroy(impl);

		free(impl);
	}
}
