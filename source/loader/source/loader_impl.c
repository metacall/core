/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

/* -- Headers -- */

#include <loader/loader_impl.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_context.h>

#include <adt/adt_hash_map.h>

#include <dynlink/dynlink.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

/* -- Forward Declarations -- */

struct loader_handle_impl_type;

struct loader_impl_type;

/* -- Type Definitions -- */

typedef struct loader_handle_impl_type * loader_handle_impl;

typedef struct loader_impl_type * loader_impl;

/* -- Member Data -- */

struct loader_handle_impl_type
{
	loader_naming_name name;
	loader_handle module;
	context ctx;
};

struct loader_impl_type
{
	loader_naming_extension extension;
	dynlink handle;
	loader_impl_interface_singleton singleton;
	hash_map handle_impl_map;
	loader_impl_data data;
	context ctx;
	hash_map type_info_map;
};

/* -- Private Methods -- */

static dynlink loader_impl_dynlink_load(const char * path, loader_naming_extension extension);

static int loader_impl_dynlink_symbol(loader_impl impl, loader_naming_extension extension, dynlink_symbol_addr * singleton_addr_ptr);

static void loader_impl_dynlink_destroy(loader_impl impl);

static int loader_impl_create_singleton(loader_impl impl, const char * path, loader_naming_extension extension);

static loader_handle_impl loader_impl_load_handle(loader_handle module, const loader_naming_name name);

static void loader_impl_destroy_handle(loader_handle_impl handle_impl);

static int loader_impl_destroy_type_map_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args);

static int loader_impl_destroy_handle_map_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args);

/* -- Methods -- */

static dynlink loader_impl_dynlink_load(const char * path, loader_naming_extension extension)
{
	const char loader_dynlink_suffix[] = "_loader";

	#define LOADER_DYNLINK_NAME_SIZE \
		(sizeof(loader_dynlink_suffix) + LOADER_NAMING_EXTENSION_SIZE)

	char loader_dynlink_name[LOADER_DYNLINK_NAME_SIZE];

	strncpy(loader_dynlink_name, extension, LOADER_DYNLINK_NAME_SIZE - 1);

	strncat(loader_dynlink_name, loader_dynlink_suffix,
		LOADER_DYNLINK_NAME_SIZE - strnlen(loader_dynlink_name, LOADER_DYNLINK_NAME_SIZE - 1) - 1);

	#undef LOADER_DYNLINK_NAME_SIZE

	log_write("metacall", LOG_LEVEL_DEBUG, "Loader: %s", loader_dynlink_name);

	return dynlink_load(path, loader_dynlink_name, DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);
}

static int loader_impl_dynlink_symbol(loader_impl impl, loader_naming_extension extension, dynlink_symbol_addr * singleton_addr_ptr)
{
	const char loader_dynlink_symbol_prefix[] = DYNLINK_SYMBOL_STR("");
	const char loader_dynlink_symbol_suffix[] = "_loader_impl_interface_singleton";

	#define LOADER_DYNLINK_SYMBOL_SIZE \
		(sizeof(loader_dynlink_symbol_prefix) + LOADER_NAMING_EXTENSION_SIZE + sizeof(loader_dynlink_symbol_suffix))

	char loader_dynlink_symbol[LOADER_DYNLINK_SYMBOL_SIZE];

	strncpy(loader_dynlink_symbol, loader_dynlink_symbol_prefix, LOADER_DYNLINK_SYMBOL_SIZE - 1);

	strncat(loader_dynlink_symbol, extension,
		LOADER_DYNLINK_SYMBOL_SIZE - strnlen(loader_dynlink_symbol, LOADER_DYNLINK_SYMBOL_SIZE - 1) - 1);

	strncat(loader_dynlink_symbol, loader_dynlink_symbol_suffix,
		LOADER_DYNLINK_SYMBOL_SIZE - strnlen(loader_dynlink_symbol, LOADER_DYNLINK_SYMBOL_SIZE - 1) - 1);

	#undef LOADER_DYNLINK_SYMBOL_SIZE

	log_write("metacall", LOG_LEVEL_DEBUG, "Loader symbol: %s", loader_dynlink_symbol);

	return dynlink_symbol(impl->handle, loader_dynlink_symbol, singleton_addr_ptr);
}

static void loader_impl_dynlink_destroy(loader_impl impl)
{
	dynlink_unload(impl->handle);
}

static int loader_impl_create_singleton(loader_impl impl, const char * path, loader_naming_extension extension)
{
	impl->handle = loader_impl_dynlink_load(path, extension);

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

loader_impl loader_impl_create(const char * path, loader_naming_extension extension)
{
	if (extension != NULL)
	{
		loader_impl impl = malloc(sizeof(struct loader_impl_type));

		if (impl != NULL)
		{
			if (loader_impl_create_singleton(impl, path, extension) == 0)
			{
				impl->handle_impl_map = hash_map_create(&hash_callback_str, &comparable_callback_str);

				if (impl->handle_impl_map != NULL)
				{
					impl->type_info_map = hash_map_create(&hash_callback_str, &comparable_callback_str);

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
		return (type)hash_map_get(impl->type_info_map, (const hash_map_key)name);
	}

	return NULL;
}

int loader_impl_type_define(loader_impl impl, const char * name, type t)
{
	if (impl != NULL && impl->type_info_map != NULL && name != NULL)
	{
		return hash_map_insert(impl->type_info_map, (const hash_map_key)name, (hash_map_value)t);
	}

	return 1;
}

static loader_handle_impl loader_impl_load_handle(loader_handle module, const loader_naming_name name)
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

static void loader_impl_destroy_handle(loader_handle_impl handle_impl)
{
	if (handle_impl != NULL)
	{
		context_destroy(handle_impl->ctx);

		free(handle_impl);
	}
}

int loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
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

int loader_impl_load_from_file(loader_impl impl, const loader_naming_path path)
{
	if (impl != NULL)
	{
		loader_impl_interface interface_impl = loader_impl_symbol(impl);

		loader_naming_name module_name;

		log_write("metacall", LOG_LEVEL_DEBUG, "Loading %s", path);

		if (interface_impl != NULL && loader_path_get_name(path, module_name) > 1)
		{
			loader_handle handle = interface_impl->load_from_file(impl, path, module_name);

			log_write("metacall", LOG_LEVEL_DEBUG, "Loader interface: %p\nLoader handle: %p", (void *)interface_impl, (void *)handle);

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

int loader_impl_load_from_memory_name(loader_impl impl, loader_naming_name name, const char * buffer, size_t size)
{
	/* TODO: Improve name with time */
	static const char format[] = "%p-%p-%" PRIuS;

	#if defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER < 1900)

		size_t length = _snprintf(NULL, 0, format, (const void *)impl, (const void *)buffer, size);

	#elif (defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER >= 1900)) || \
		defined(_BSD_SOURCE) || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 500) || \
		defined(_ISOC99_SOURCE) || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L)

		size_t length = snprintf(NULL, 0, format, (const void *)impl, (const void *)buffer, size);

	#else

		size_t length = sprintf(NULL, format, (const void *)impl, (const void *)buffer, size);

	#endif

	if (length < LOADER_NAMING_NAME_SIZE)
	{
		#if defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER < 1900)

			size_t written = _snprintf(name, length, format, (const void *)impl, (const void *)buffer, size);

		#elif (defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER >= 1900)) || \
			defined(_BSD_SOURCE) || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 500) || \
			defined(_ISOC99_SOURCE) || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L)

			size_t written = snprintf(name, length, format, (const void *)impl, (const void *)buffer, size);

		#else

			size_t written = sprintf(name, format, (const void *)impl, (const void *)buffer, size);

		#endif

		if (written == length)
		{
			name[length] = '\0';

			return 0;
		}
	}
	
	return 1;
}

int loader_impl_load_from_memory(loader_impl impl, const loader_naming_extension extension, const char * buffer, size_t size)
{
	if (impl != NULL && buffer != NULL && size > 0)
	{
		loader_impl_interface interface_impl = loader_impl_symbol(impl);

		log_write("metacall", LOG_LEVEL_DEBUG, "Loading from memory %.10s...", buffer);

		if (interface_impl != NULL)
		{
			loader_naming_name name;

			loader_handle handle = NULL;

			if (loader_impl_load_from_memory_name(impl, name, buffer, size) != 0)
			{
				return 1;
			}

			handle = interface_impl->load_from_memory(impl, name, extension, buffer, size);

			log_write("metacall", LOG_LEVEL_DEBUG, "Loader interface: %p\nLoader handle: %p", (void *)interface_impl, (void *)handle);

			if (handle != NULL)
			{
				loader_handle_impl handle_impl = loader_impl_load_handle(handle, name);

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


static int loader_impl_destroy_type_map_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args)
{
	if (map != NULL && key != NULL && val != NULL && args == NULL)
	{
		type t = val;

		type_destroy(t);

		return 0;
	}

	return 1;
}

static int loader_impl_destroy_handle_map_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args)
{
	if (map != NULL && key != NULL && val != NULL && args == NULL)
	{
		loader_handle_impl handle_impl = val;

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
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid loader implementation (%s) interface destruction <%p>", impl->extension, interface_impl->destroy);
		}

		hash_map_iterate(impl->handle_impl_map, &loader_impl_destroy_handle_map_cb_iterate, NULL);

		hash_map_destroy(impl->handle_impl_map);

		context_destroy(impl->ctx);

		loader_impl_dynlink_destroy(impl);

		free(impl);
	}
}
