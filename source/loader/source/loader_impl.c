/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

 /* -- Headers -- */

#include <loader/loader_impl.h>
#include <loader/loader_env.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_context.h>

#include <adt/adt_hash.h>
#include <adt/adt_set.h>
#include <adt/adt_vector.h>

#include <dynlink/dynlink.h>

#include <format/format_print.h>

#include <log/log.h>

#include <configuration/configuration.h>

#include <stdlib.h>
#include <string.h>

/* -- Definitions -- */

#define LOADER_IMPL_FUNCTION_INIT "__metacall_initialize__"
#define LOADER_IMPL_FUNCTION_FINI "__metacall_finalize__"

/* -- Forward Declarations -- */

struct loader_handle_impl_type;

struct loader_impl_metadata_cb_iterator_type;

/* -- Type Definitions -- */

typedef struct loader_handle_impl_type * loader_handle_impl;

typedef struct loader_impl_metadata_cb_iterator_type * loader_impl_metadata_cb_iterator;

/* -- Member Data -- */

struct loader_impl_type
{
	int init;
	loader_naming_tag tag;
	dynlink handle;
	loader_impl_interface_singleton singleton;
	set handle_impl_map;
	loader_impl_data data;
	context ctx;
	set type_info_map;
	loader_host host;
	void * options;
	set exec_path_map;
};

struct loader_handle_impl_type
{
	loader_impl impl;
	loader_naming_name name;
	loader_handle module;
	context ctx;
};

struct loader_impl_metadata_cb_iterator_type
{
	size_t iterator;
	value * values;
};

/* -- Private Methods -- */

static int loader_impl_initialize(loader_impl impl);

static dynlink loader_impl_dynlink_load(const char * path, const loader_naming_tag tag);

static int loader_impl_dynlink_symbol(loader_impl impl, const loader_naming_tag tag, dynlink_symbol_addr * singleton_addr_ptr);

static void loader_impl_dynlink_destroy(loader_impl impl);

static int loader_impl_create_singleton(loader_impl impl, const char * path, const loader_naming_tag tag);

static loader_handle_impl loader_impl_load_handle(loader_impl impl, loader_handle module, const loader_naming_name name);

static int loader_impl_function_hook_call(context ctx, const char func_name[]);

static value loader_impl_metadata_handle_name(loader_handle_impl handle_impl);

static value loader_impl_metadata_handle_context(loader_handle_impl handle_impl);

static value loader_impl_metadata_handle(loader_handle_impl handle_impl);

static int loader_impl_metadata_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

static void loader_impl_destroy_handle(loader_handle_impl handle_impl, int unlink);

static int loader_impl_destroy_type_map_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

static int loader_impl_destroy_handle_map_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

/* -- Methods -- */

dynlink loader_impl_dynlink_load(const char * path, const loader_naming_tag tag)
{
	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
		const char loader_dynlink_suffix[] = "_loaderd";
	#else
		const char loader_dynlink_suffix[] = "_loader";
	#endif

	#define LOADER_DYNLINK_NAME_SIZE \
		(sizeof(loader_dynlink_suffix) + LOADER_NAMING_TAG_SIZE)

	char loader_dynlink_name[LOADER_DYNLINK_NAME_SIZE];

	strncpy(loader_dynlink_name, tag, LOADER_DYNLINK_NAME_SIZE);

	strncat(loader_dynlink_name, loader_dynlink_suffix,
		LOADER_DYNLINK_NAME_SIZE - strnlen(loader_dynlink_name, LOADER_DYNLINK_NAME_SIZE - 1) - 1);

	#undef LOADER_DYNLINK_NAME_SIZE

	log_write("metacall", LOG_LEVEL_DEBUG, "Loader: %s", loader_dynlink_name);

	return dynlink_load(path, loader_dynlink_name, DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);
}

int loader_impl_dynlink_symbol(loader_impl impl, const loader_naming_tag tag, dynlink_symbol_addr * singleton_addr_ptr)
{
	const char loader_dynlink_symbol_prefix[] = DYNLINK_SYMBOL_STR("");
	const char loader_dynlink_symbol_suffix[] = "_loader_impl_interface_singleton";

	#define LOADER_DYNLINK_SYMBOL_SIZE \
		(sizeof(loader_dynlink_symbol_prefix) + LOADER_NAMING_TAG_SIZE + sizeof(loader_dynlink_symbol_suffix))

	char loader_dynlink_symbol[LOADER_DYNLINK_SYMBOL_SIZE];

	strncpy(loader_dynlink_symbol, loader_dynlink_symbol_prefix, LOADER_DYNLINK_SYMBOL_SIZE);

	strncat(loader_dynlink_symbol, tag,
		LOADER_DYNLINK_SYMBOL_SIZE - strnlen(loader_dynlink_symbol, LOADER_DYNLINK_SYMBOL_SIZE - 1) - 1);

	strncat(loader_dynlink_symbol, loader_dynlink_symbol_suffix,
		LOADER_DYNLINK_SYMBOL_SIZE - strnlen(loader_dynlink_symbol, LOADER_DYNLINK_SYMBOL_SIZE - 1) - 1);

	#undef LOADER_DYNLINK_SYMBOL_SIZE

	log_write("metacall", LOG_LEVEL_DEBUG, "Loader symbol: %s", loader_dynlink_symbol);

	return dynlink_symbol(impl->handle, loader_dynlink_symbol, singleton_addr_ptr);
}

void loader_impl_dynlink_destroy(loader_impl impl)
{
	dynlink_unload(impl->handle);
}

int loader_impl_create_singleton(loader_impl impl, const char * path, const loader_naming_tag tag)
{
	impl->handle = loader_impl_dynlink_load(path, tag);

	if (impl->handle != NULL)
	{
		dynlink_symbol_addr singleton_addr;

		if (loader_impl_dynlink_symbol(impl, tag, &singleton_addr) == 0)
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

loader_impl loader_impl_create_proxy(loader_host host)
{
	loader_impl impl = malloc(sizeof(struct loader_impl_type));

	memset(impl, 0, sizeof(struct loader_impl_type));

	impl->init = 0; /* Do not call singleton initialize */
	impl->host = host;
	impl->options = NULL;

	if (impl != NULL)
	{
		impl->handle_impl_map = set_create(&hash_callback_str, &comparable_callback_str);

		if (impl->handle_impl_map != NULL)
		{
			impl->type_info_map = set_create(&hash_callback_str, &comparable_callback_str);

			if (impl->type_info_map != NULL)
			{
				static char loader_host_proxy_name[] = LOADER_HOST_PROXY_NAME;

				impl->ctx = context_create(loader_host_proxy_name);

				if (impl->ctx != NULL)
				{
					strncpy(impl->tag, loader_host_proxy_name, LOADER_NAMING_TAG_SIZE);

					return impl;
				}

				set_destroy(impl->type_info_map);
			}

			set_destroy(impl->handle_impl_map);
		}
	}

	return NULL;
}

void loader_impl_configuration(loader_impl impl, configuration config)
{
	value execution_paths_value = configuration_value(config, "execution_paths");

	size_t size = value_type_count(execution_paths_value);

	if (execution_paths_value != NULL)
	{
		value * execution_paths_array = value_to_array(execution_paths_value);

		if (execution_paths_array != NULL)
		{
			size_t iterator;

			for (iterator = 0; iterator < size; ++iterator)
			{
				if (execution_paths_array[iterator] != NULL)
				{
					const char * str = value_to_string(execution_paths_array[iterator]);

					if (str != NULL)
					{
						loader_naming_path execution_path;

						strncpy(execution_path, str, LOADER_NAMING_PATH_SIZE);

						impl->singleton()->execution_path(impl, execution_path);
					}
				}
			}
		}
	}
}

int loader_impl_initialize(loader_impl impl)
{
	char configuration_key[0xFF];

	configuration config;

	const char * script_path = NULL;

	vector paths;

	if (impl->init == 0)
	{
		return 0;
	}

	strcpy(configuration_key, impl->tag);

	strcat(configuration_key, "_loader");

	config = configuration_scope(configuration_key);

	impl->data = impl->singleton()->initialize(impl, config, impl->host);

	if (impl->data == NULL)
	{
		context_destroy(impl->ctx);

		return 1;
	}

	impl->init = 0;

	if (config != NULL)
	{
		loader_impl_configuration(impl, config);
	}

	script_path = loader_env_script_path();

	if (script_path != NULL)
	{
		if (loader_impl_execution_path(impl, script_path) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Error when loading path %s", script_path);
		}
	}

	paths = set_get(impl->exec_path_map, (const set_key)impl->tag);

	/* Load all execution paths */
	if (paths != NULL)
	{
		size_t iterator, size = vector_size(paths);

		for (iterator = 0; iterator < size; ++iterator)
		{
			char * path = vector_at(paths, iterator);

			if (loader_impl_execution_path(impl, path) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Loader (%s) failed to load path: %s", impl->tag, path);
			}
		}
	}

	return 0;
}

int loader_impl_is_initialized(loader_impl impl)
{
	return impl->init;
}

loader_impl loader_impl_create(const char * path, const loader_naming_tag tag, loader_host host)
{
	if (tag != NULL)
	{
		loader_impl impl = malloc(sizeof(struct loader_impl_type));

		if (impl == NULL)
		{
			return NULL;
		}

		impl->init = 1;
		impl->host = host;
		impl->options = NULL;

		if (loader_impl_create_singleton(impl, path, tag) == 0)
		{
			impl->handle_impl_map = set_create(&hash_callback_str, &comparable_callback_str);

			if (impl->handle_impl_map != NULL)
			{
				impl->type_info_map = set_create(&hash_callback_str, &comparable_callback_str);

				if (impl->type_info_map != NULL)
				{
					impl->ctx = context_create(tag);

					if (impl->ctx != NULL)
					{
						strncpy(impl->tag, tag, LOADER_NAMING_TAG_SIZE);

						impl->exec_path_map = set_create(&hash_callback_str, &comparable_callback_str);

						if (impl->exec_path_map != NULL)
						{
							return impl;
						}

						context_destroy(impl->ctx);
					}

					set_destroy(impl->type_info_map);
				}

				set_destroy(impl->handle_impl_map);
			}
		}

		free(impl);
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

loader_naming_tag * loader_impl_tag(loader_impl impl)
{
	if (impl != NULL)
	{
		return &impl->tag;
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
		return (type)set_get(impl->type_info_map, (const set_key)name);
	}

	return NULL;
}

int loader_impl_type_define(loader_impl impl, const char * name, type t)
{
	if (impl != NULL && impl->type_info_map != NULL && name != NULL)
	{
		return set_insert(impl->type_info_map, (const set_key)name, (set_value)t);
	}

	return 1;
}

loader_handle_impl loader_impl_load_handle(loader_impl impl, loader_handle module, const loader_naming_name name)
{
	loader_handle_impl handle_impl = malloc(sizeof(struct loader_handle_impl_type));

	if (handle_impl != NULL)
	{
		handle_impl->impl = impl;

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

void loader_impl_destroy_handle(loader_handle_impl handle_impl, int unlink)
{
	if (handle_impl != NULL)
	{
		static const char func_fini_name[] = LOADER_IMPL_FUNCTION_FINI;

		loader_impl_interface interface_impl = loader_impl_symbol(handle_impl->impl);

		if (handle_impl->impl->init == 0)
		{
			if (loader_impl_function_hook_call(handle_impl->ctx, func_fini_name) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Error when calling destructor from handle impl: %p (%s)", (void *)handle_impl, func_fini_name);
			}

			if (interface_impl->clear(handle_impl->impl, handle_impl->module) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Error when clearing handle impl: %p", (void *)handle_impl);
			}
		}

		if (unlink == 0)
		{
			context_remove(handle_impl->impl->ctx, handle_impl->ctx);
		}

		context_destroy(handle_impl->ctx);

		free(handle_impl);
	}
}

int loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	if (impl != NULL)
	{
		if (impl->init == 0)
		{
			/* If loader is initialized, load the execution path */
			loader_impl_interface interface_impl = loader_impl_symbol(impl);

			if (interface_impl != NULL)
			{
				return interface_impl->execution_path(impl, path);
			}
		}
		else
		{
			/* If loader is not initialized, store the execution path for later use */
			vector paths = set_get(impl->exec_path_map, (set_key)impl->tag);

			if (paths == NULL)
			{
				paths = vector_create(sizeof(char) * LOADER_NAMING_PATH_SIZE);

				if (paths == NULL)
				{
					return 1;
				}

				if (set_insert(impl->exec_path_map, (set_key)impl->tag, paths) != 0)
				{
					vector_destroy(paths);

					return 1;
				}
			}

			vector_push_back(paths, (void *)path);

			return 0;
		}
	}

	return 1;
}

int loader_impl_function_hook_call(context ctx, const char func_name[])
{
	scope sp = context_scope(ctx);

	value val = scope_get(sp, func_name);

	function func_init = NULL;

	if (val != NULL)
	{
		func_init = value_to_function(val);
	}

	if (func_init != NULL)
	{
		void * null_args[1] = { NULL };

		function_return ret = function_call(func_init, null_args, 0);

		if (ret != NULL)
		{
			int result = value_to_int(ret);

			value_destroy(ret);

			return result;
		}
	}

	return 0;
}

int loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size, void ** handle_ptr)
{
	if (impl != NULL)
	{
		loader_impl_interface interface_impl = loader_impl_symbol(impl);

		size_t iterator;

		for (iterator = 0; iterator < size; ++iterator)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Loading %s", paths[iterator]);
		}

		if (interface_impl != NULL)
		{
			loader_handle handle;

			loader_naming_name module_name;

			if (loader_impl_initialize(impl) != 0)
			{
				return 1;
			}

			/* TODO: Refactor loader_path_get_name from path 0 (for avoiding collisions of scripts): */
			/*
			int loader_impl_load_from_file_ex(loader_impl impl, const loader_naming_path paths[], size_t size, const char * name, size_t length, void ** handle_ptr)
			{
				...
				if (name == NULL)
				{
					loader_path_get_name(paths[0], module_name)
				}
				else
				{
					// TODO: Name must be a generated UUID to avoid collisions
					strncpy(module_name, name, length);
				}
			}
			*/

			if (loader_path_get_name(paths[0], module_name) > 1 && loader_impl_get_handle(impl, module_name) != NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Load from file handle failed, handle with name %s already loaded", module_name);

				return 1;
			}

			handle = interface_impl->load_from_file(impl, paths, size);

			log_write("metacall", LOG_LEVEL_DEBUG, "Loader interface: %p\nLoader handle: %p", (void *)interface_impl, (void *)handle);

			if (handle != NULL)
			{
				loader_handle_impl handle_impl = loader_impl_load_handle(impl, handle, module_name);

				log_write("metacall", LOG_LEVEL_DEBUG, "Loader handle impl: %p", (void *)handle_impl);

				if (handle_impl != NULL)
				{
					if (set_insert(impl->handle_impl_map, handle_impl->name, handle_impl) == 0)
					{
						if (interface_impl->discover(impl, handle_impl->module, handle_impl->ctx) == 0)
						{
							// TODO: Check if is contained in the context
							// if (context_contains(impl->ctx, handle_impl->ctx) == 0 && context_append...)
							if (context_append(impl->ctx, handle_impl->ctx) == 0)
							{
								static const char func_init_name[] = LOADER_IMPL_FUNCTION_INIT;

								int result = loader_impl_function_hook_call(impl->ctx, func_init_name);

								if (result != 0)
								{
									log_write("metacall", LOG_LEVEL_ERROR, "Error when calling to init hook function (" LOADER_IMPL_FUNCTION_INIT ") of handle: %s", module_name);
								}

								if (handle_ptr != NULL)
								{
									*handle_ptr = handle_impl;
								}

								return result;
							}
						}

						set_remove(impl->handle_impl_map, handle_impl->name);
					}

					log_write("metacall", LOG_LEVEL_ERROR, "Error when loading handle: %s", module_name);

					loader_impl_destroy_handle(handle_impl, 1);
				}
			}
		}
	}

	return 1;
}

int loader_impl_load_from_memory_name(loader_impl impl, loader_naming_name name, const char * buffer, size_t size)
{
	/* TODO: Improve name with time or uuid */
	static const char format[] = "%p-%p-%" PRIuS "-%u";

	hash h = hash_callback_str((const hash_key)buffer);

	size_t length = snprintf(NULL, 0, format, (const void *)impl, (const void *)buffer, size, (unsigned int)h);

	if (length > 0 && length < LOADER_NAMING_NAME_SIZE)
	{
		size_t written = snprintf(name, length + 1, format, (const void *)impl, (const void *)buffer, size, (unsigned int)h);

		if (written == length)
		{
			return 0;
		}
	}

	return 1;
}

int loader_impl_load_from_memory(loader_impl impl, const char * buffer, size_t size, void ** handle_ptr)
{
	if (impl != NULL && buffer != NULL && size > 0)
	{
		loader_impl_interface interface_impl = loader_impl_symbol(impl);

		log_write("metacall", LOG_LEVEL_DEBUG, "Loading from memory %.10s...", buffer);

		if (interface_impl != NULL)
		{
			loader_naming_name name;

			loader_handle handle = NULL;

			if (loader_impl_initialize(impl) != 0)
			{
				return 1;
			}

			if (loader_impl_load_from_memory_name(impl, name, buffer, size) != 0)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Load from memory handle failed, name could not be generated correctly");

				return 1;
			}

			if (loader_impl_get_handle(impl, name) != NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Load from memory handle failed, handle with name %s already loaded", name);

				return 1;
			}

			handle = interface_impl->load_from_memory(impl, name, buffer, size);

			log_write("metacall", LOG_LEVEL_DEBUG, "Loader interface: %p\nLoader handle: %p", (void *)interface_impl, (void *)handle);

			if (handle != NULL)
			{
				loader_handle_impl handle_impl = loader_impl_load_handle(impl, handle, name);

				if (handle_impl != NULL)
				{
					if (set_insert(impl->handle_impl_map, handle_impl->name, handle_impl) == 0)
					{
						if (interface_impl->discover(impl, handle_impl->module, handle_impl->ctx) == 0)
						{
							// TODO: Check if is contained in the context
							// if (context_contains(impl->ctx, handle_impl->ctx) == 0 && context_append...)
							if (context_append(impl->ctx, handle_impl->ctx) == 0)
							{
								static const char func_init_name[] = LOADER_IMPL_FUNCTION_INIT;

								int result = loader_impl_function_hook_call(impl->ctx, func_init_name);

								if (result != 0)
								{
									log_write("metacall", LOG_LEVEL_ERROR, "Error when calling to init hook function (" LOADER_IMPL_FUNCTION_INIT ") of handle: %s", name);
								}

								if (handle_ptr != NULL)
								{
									*handle_ptr = handle_impl;
								}

								return result;
							}
						}

						set_remove(impl->handle_impl_map, handle_impl->name);
					}

					log_write("metacall", LOG_LEVEL_ERROR, "Error when loading handle: %s", name);

					loader_impl_destroy_handle(handle_impl, 1);
				}
			}
		}
	}

	return 1;
}

int loader_impl_load_from_package(loader_impl impl, const loader_naming_path path, void ** handle_ptr)
{
	if (impl != NULL)
	{
		loader_impl_interface interface_impl = loader_impl_symbol(impl);

		loader_naming_name package_name;

		if (interface_impl != NULL && loader_path_get_name(path, package_name) > 1)
		{
			loader_handle handle;

			if (loader_impl_initialize(impl) != 0)
			{
				return 1;
			}

			if (loader_impl_get_handle(impl, package_name) != NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Load from package handle failed, handle with name %s already loaded", package_name);

				return 1;
			}

			handle = interface_impl->load_from_package(impl, path);

			log_write("metacall", LOG_LEVEL_DEBUG, "Loader interface: %p\nLoader handle: %p", (void *)interface_impl, (void *)handle);

			if (handle != NULL)
			{
				loader_handle_impl handle_impl = loader_impl_load_handle(impl, handle, package_name);

				if (handle_impl != NULL)
				{
					if (set_insert(impl->handle_impl_map, handle_impl->name, handle_impl) == 0)
					{
						if (interface_impl->discover(impl, handle_impl->module, handle_impl->ctx) == 0)
						{
							// TODO: Check if is contained in the context
							// if (context_contains(impl->ctx, handle_impl->ctx) == 0 && context_append...)
							if (context_append(impl->ctx, handle_impl->ctx) == 0)
							{
								static const char func_init_name[] = LOADER_IMPL_FUNCTION_INIT;

								int result = loader_impl_function_hook_call(impl->ctx, func_init_name);

								if (result != 0)
								{
									log_write("metacall", LOG_LEVEL_ERROR, "Error when calling to init hook function (" LOADER_IMPL_FUNCTION_INIT ") of handle: %s", package_name);
								}

								if (handle_ptr != NULL)
								{
									*handle_ptr = handle_impl;
								}

								return result;
							}
						}

						set_remove(impl->handle_impl_map, handle_impl->name);
					}

					log_write("metacall", LOG_LEVEL_ERROR, "Error when loading handle: %s", (void *)package_name);

					loader_impl_destroy_handle(handle_impl, 1);
				}
			}
		}
	}

	return 1;
}

void * loader_impl_get_handle(loader_impl impl, const char * name)
{
	if (impl != NULL && name != NULL)
	{
		return (void *)set_get(impl->handle_impl_map, (set_key)name);
	}

	return NULL;
}

void loader_impl_set_options(loader_impl impl, void * options)
{
	if (impl != NULL && options != NULL)
	{
		impl->options = options;
	}
}

void * loader_impl_get_options(loader_impl impl)
{
	if (impl != NULL)
	{
		return impl->options;
	}

	return NULL;
}

const char * loader_impl_handle_id(void * handle)
{
	loader_handle_impl handle_impl = handle;

	return handle_impl->name;
}

value loader_impl_handle_export(void * handle)
{
	loader_handle_impl handle_impl = handle;

	return scope_export(context_scope(handle_impl->ctx));
}

value loader_impl_metadata_handle_name(loader_handle_impl handle_impl)
{
	static const char name[] = "name";

	value * v_ptr, v = value_create_array(NULL, 2);

	if (v == NULL)
	{
		return NULL;
	}

	v_ptr = value_to_array(v);

	v_ptr[0] = value_create_string(name, sizeof(name) - 1);

	if (v_ptr[0] == NULL)
	{
		value_type_destroy(v);

		return NULL;
	}

	v_ptr[1] = value_create_string(handle_impl->name, strlen(handle_impl->name));

	if (v_ptr[1] == NULL)
	{
		value_type_destroy(v);

		return NULL;
	}

	return v;
}

value loader_impl_metadata_handle_context(loader_handle_impl handle_impl)
{
	static const char name[] = "scope";

	value * v_ptr, v = value_create_array(NULL, 2);

	if (v == NULL)
	{
		return NULL;
	}

	v_ptr = value_to_array(v);

	v_ptr[0] = value_create_string(name, sizeof(name) - 1);

	if (v_ptr[0] == NULL)
	{
		value_type_destroy(v);

		return NULL;
	}

	v_ptr[1] = scope_metadata(context_scope(handle_impl->ctx));

	if (v_ptr[1] == NULL)
	{
		value_type_destroy(v);

		return NULL;
	}

	return v;
}

value loader_impl_metadata_handle(loader_handle_impl handle_impl)
{
	value * v_ptr, v = value_create_map(NULL, 2);

	if (v == NULL)
	{
		return NULL;
	}

	v_ptr = value_to_map(v);

	v_ptr[0] = loader_impl_metadata_handle_name(handle_impl);

	if (v_ptr[0] == NULL)
	{
		value_type_destroy(v);

		return NULL;
	}

	v_ptr[1] = loader_impl_metadata_handle_context(handle_impl);

	if (v_ptr[1] == NULL)
	{
		value_type_destroy(v);

		return NULL;
	}

	return v;
}

int loader_impl_metadata_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	loader_impl_metadata_cb_iterator metadata_iterator = (loader_impl_metadata_cb_iterator)args;

	(void)s;
	(void)key;

	metadata_iterator->values[metadata_iterator->iterator] = loader_impl_metadata_handle((loader_handle_impl)val);

	if (metadata_iterator->values[metadata_iterator->iterator] != NULL)
	{
		++metadata_iterator->iterator;
	}

	return 0;
}

value loader_impl_metadata(loader_impl impl)
{
	struct loader_impl_metadata_cb_iterator_type metadata_iterator;

	value v = value_create_array(NULL, set_size(impl->handle_impl_map));

	if (v == NULL)
	{
		return NULL;
	}

	metadata_iterator.iterator = 0;
	metadata_iterator.values = value_to_array(v);

	set_iterate(impl->handle_impl_map, &loader_impl_metadata_cb_iterate, (set_cb_iterate_args)&metadata_iterator);

	return v;
}

int loader_impl_clear(void * handle)
{
	if (handle != NULL)
	{
		loader_handle_impl handle_impl = handle;

		loader_impl impl = handle_impl->impl;

		int result = !(set_remove(impl->handle_impl_map, (set_key)(handle_impl->name)) == handle_impl);

		loader_impl_destroy_handle(handle_impl, 0);

		return result;
	}

	return 1;
}

int loader_impl_destroy_type_map_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	(void)s;
	(void)key;
	(void)args;

	if (val != NULL)
	{
		type t = val;

		type_destroy(t);

		return 0;
	}

	return 1;
}

int loader_impl_destroy_handle_map_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	(void)s;
	(void)key;
	(void)args;

	if (val != NULL)
	{
		loader_handle_impl handle_impl = val;

		loader_impl_destroy_handle(handle_impl, 0);

		return 0;
	}

	return 1;
}

int loader_impl_destroy_exec_path_map_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	(void)s;
	(void)key;
	(void)args;

	if (val != NULL)
	{
		vector paths = val;

		vector_destroy(paths);
	}

	return 0;
}

void loader_impl_destroy(loader_impl impl)
{
	if (impl != NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "Destroy loader implementation %s", impl->tag);

		set_iterate(impl->handle_impl_map, &loader_impl_destroy_handle_map_cb_iterate, NULL);

		set_iterate(impl->type_info_map, &loader_impl_destroy_type_map_cb_iterate, NULL);

		set_destroy(impl->type_info_map);

		if (impl->init == 0)
		{
			loader_impl_interface interface_impl = loader_impl_symbol(impl);

			if (interface_impl != NULL && interface_impl->destroy(impl) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid loader implementation (%s) interface destruction <%p>", impl->tag, interface_impl->destroy);
			}

			impl->init = 1;
		}

		set_iterate(impl->exec_path_map, &loader_impl_destroy_exec_path_map_cb_iterate, NULL);

		set_destroy(impl->exec_path_map);

		set_destroy(impl->handle_impl_map);

		context_destroy(impl->ctx);

		loader_impl_dynlink_destroy(impl);

		free(impl->host);

		free(impl);
	}
}
