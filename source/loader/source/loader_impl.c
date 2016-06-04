/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#include <loader/loader_impl.h>

#include <reflect/type.h>
#include <reflect/context.h>

#include <adt/hash_map.h>
#include <adt/hash_map_str.h>

#include <dynlink/dynlink.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct loader_handle_impl_type
{
	loader_naming_name name;
	loader_handle module;
	context ctx;

} * loader_handle_impl;

typedef struct loader_impl_type
{
	loader_naming_extension extension;
	dynlink handle;
	loader_impl_interface_singleton singleton;
	hash_map handle_impl_map;
	loader_impl_data data;
	context ctx;
	hash_map type_info_map;

} * loader_impl;

dynlink loader_impl_dynlink_load(loader_naming_extension extension)
{
	const char loader_dynlink_suffix[] = "_loader";

	#define LOADER_DYNLINK_NAME_SIZE \
		(sizeof(loader_dynlink_suffix) + LOADER_NAMING_EXTENSION_SIZE)

	char loader_dynlink_name[LOADER_DYNLINK_NAME_SIZE];

	strncpy(loader_dynlink_name, extension, LOADER_NAMING_EXTENSION_SIZE);

	strncat(loader_dynlink_name, loader_dynlink_suffix, LOADER_DYNLINK_NAME_SIZE);

	#undef LOADER_DYNLINK_NAME_SIZE

	printf("Loader: %s\n", loader_dynlink_name);

	return dynlink_load(loader_dynlink_name, DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);
}

int loader_impl_dynlink_symbol(loader_impl impl, loader_naming_extension extension, dynlink_symbol_addr * singleton_addr_ptr)
{
	const char loader_dynlink_symbol_prefix[] = DYNLINK_SYMBOL_STR("");
	const char loader_dynlink_symbol_suffix[] = "_loader_impl_interface_singleton";

	#define LOADER_DYNLINK_SYMBOL_SIZE \
		(sizeof(loader_dynlink_symbol_prefix) + LOADER_NAMING_EXTENSION_SIZE + sizeof(loader_dynlink_symbol_suffix))

	char loader_dynlink_symbol[LOADER_DYNLINK_SYMBOL_SIZE];

	strncpy(loader_dynlink_symbol, loader_dynlink_symbol_prefix, sizeof(loader_dynlink_symbol_prefix));

	strncat(loader_dynlink_symbol, extension, LOADER_DYNLINK_SYMBOL_SIZE);

	strncat(loader_dynlink_symbol, loader_dynlink_symbol_suffix, sizeof(loader_dynlink_symbol_suffix));

	#undef LOADER_DYNLINK_SYMBOL_SIZE

	printf("Loader symbol: %s\n", loader_dynlink_symbol);

	return dynlink_symbol(impl->handle, loader_dynlink_symbol, singleton_addr_ptr);
}

void loader_impl_dynlink_destroy(loader_impl impl)
{
	dynlink_unload(impl->handle);
}

int loader_impl_create_singleton(loader_impl impl, loader_naming_extension extension)
{
	impl->handle = loader_impl_dynlink_load(extension);

	if (impl->handle != NULL)
	{
		dynlink_symbol_addr singleton_addr;

		if (loader_impl_dynlink_symbol(impl, extension, &singleton_addr) == 0)
		{
			impl->singleton = (loader_impl_interface_singleton)DYNLINK_SYMBOL_GET(singleton_addr);

			if (impl->singleton != NULL)
			{
				return 0;
			}
		}

		loader_impl_dynlink_destroy(impl);
	}

	return 1;
}

loader_impl loader_impl_create(loader_naming_extension extension)
{
	if (extension != NULL)
	{
		loader_impl impl = malloc(sizeof(struct loader_impl_type));

		if (impl != NULL)
		{
			if (loader_impl_create_singleton(impl, extension) == 0)
			{
				impl->handle_impl_map = hash_map_create(&hash_map_cb_hash_str, &hash_map_cb_compare_str);

				if (impl->handle_impl_map != NULL)
				{
					impl->type_info_map = hash_map_create(&hash_map_cb_hash_str, &hash_map_cb_compare_str);

					if (impl->type_info_map != NULL)
					{
						impl->ctx = context_create(extension);

						if (impl->ctx != NULL)
						{
							impl->data = impl->singleton()->initialize(impl);

							if (impl->data != NULL)
							{
								strncpy(impl->extension, extension, LOADER_NAMING_EXTENSION_SIZE);

								return impl;
							}

							context_destroy(impl->ctx);
						}

						hash_map_destroy(impl->type_info_map);
					}

					hash_map_destroy(impl->handle_impl_map);
				}
			}
		}
	}

	return NULL;
}

loader_impl_data loader_impl_get(loader_impl impl)
{
	if (impl != NULL)
	{
		return impl->data;
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

context loader_impl_context(loader_impl impl)
{
	if (impl != NULL)
	{
		return impl->ctx;
	}

	return NULL;
}

type loader_impl_type(loader_impl impl, const char * name)
{
	if (impl != NULL && impl->type_info_map != NULL && name != NULL)
	{
		return (type)hash_map_get(impl->type_info_map, (hash_map_key)name);
	}

	return NULL;
}

int loader_impl_type_define(loader_impl impl, const char * name, type t)
{
	if (impl != NULL && impl->type_info_map != NULL && name != NULL)
	{
		return hash_map_insert(impl->type_info_map, (hash_map_key)name, (hash_map_value)t);
	}

	return 1;
}

loader_handle_impl loader_impl_load_handle(loader_handle module, loader_naming_name name)
{
	loader_handle_impl handle_impl = malloc(sizeof(struct loader_handle_impl_type));

	if (handle_impl != NULL)
	{
		strncpy(handle_impl->name, name, LOADER_NAMING_NAME_SIZE);

		handle_impl->module = module;

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

int loader_impl_execution_path(loader_impl impl, loader_naming_path path)
{
	if (impl != NULL)
	{
		loader_impl_interface interface_impl = loader_impl_symbol(impl);

		if (interface_impl != NULL)
		{
			return interface_impl->execution_path(impl, path);
		}
	}

	return 1;
}

int loader_impl_load(loader_impl impl, loader_naming_path path)
{
	if (impl != NULL)
	{
		loader_impl_interface interface_impl = loader_impl_symbol(impl);

		loader_naming_name module_name;

		printf("Loading %s\n", path);

		if (interface_impl != NULL && loader_naming_get_name(path, module_name) > 1)
		{
			loader_handle handle = interface_impl->load(impl, path, module_name);

			printf("Loader interface: %p\nLoader handle: %p\n", (void *)interface_impl, (void *)handle);

			if (handle != NULL)
			{
				loader_handle_impl handle_impl = loader_impl_load_handle(handle, module_name);

				if (handle_impl != NULL)
				{
					if (hash_map_insert(impl->handle_impl_map, handle_impl->name, handle_impl) == 0)
					{
						if (interface_impl->discover(impl, handle_impl->module, handle_impl->ctx) == 0)
						{
							if (context_append(impl->ctx, handle_impl->ctx) == 0)
							{
								return 0;
							}
						}
					}

					loader_impl_destroy_handle(handle_impl);
				}

				if (interface_impl->clear(impl, handle) != 0)
				{
					return 1;
				}
			}
		}
	}

	return 1;
}

int loader_impl_destroy_type_map_cb_iterate(hash_map map, hash_map_key key, hash_map_value value, hash_map_cb_iterate_args args)
{
	if (map != NULL && key != NULL && value != NULL && args == NULL)
	{
		type t = value;

		type_destroy(t);

		return 0;
	}

	return 1;
}

int loader_impl_destroy_handle_map_cb_iterate(hash_map map, hash_map_key key, hash_map_value value, hash_map_cb_iterate_args args)
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

		hash_map_iterate(impl->type_info_map, &loader_impl_destroy_type_map_cb_iterate, NULL);

		hash_map_destroy(impl->type_info_map);

		if (interface_impl->destroy(impl) != 0)
		{
			/* error */
		}

		hash_map_iterate(impl->handle_impl_map, &loader_impl_destroy_handle_map_cb_iterate, NULL);

		hash_map_destroy(impl->handle_impl_map);

		context_destroy(impl->ctx);

		loader_impl_dynlink_destroy(impl);

		free(impl);
	}
}
