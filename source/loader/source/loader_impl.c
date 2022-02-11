/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

/* -- Headers -- */

#include <loader/loader_env.h>
#include <loader/loader_impl.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_type.h>

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

#if defined(WIN32) || defined(_WIN32) ||            \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)

	#define LOADER_PATH_DELIMITER ';'

#else
	#define LOADER_PATH_DELIMITER ':'
#endif

/* -- Forward Declarations -- */

struct loader_handle_impl_type;

struct loader_impl_metadata_cb_iterator_type;

/* -- Type Definitions -- */

typedef struct loader_handle_impl_type *loader_handle_impl;

typedef struct loader_impl_metadata_cb_iterator_type *loader_impl_metadata_cb_iterator;

/* -- Member Data -- */

struct loader_impl_type
{
	int init;								   /* Flag for checking if the loader is initialized */
	loader_tag tag;							   /* Reference the tag of the loader (prefix), usually: py, node, rb... */
	dynlink handle;							   /* Reference to the loaded shared library */
	loader_impl_interface_singleton singleton; /* Virtual table for the loader plugin */
	set handle_impl_map;					   /* Indexes handles by path */
	loader_impl_data data;					   /* Derived metadata provided by the loader, usually contains the data of the VM, Interpreter or JIT */
	context ctx;							   /* Contains the objects, classes and functions loaded in the global scope of each loader */
	set type_info_map;						   /* Stores a set indexed by type name of all of the types existing in the loader (global scope (TODO: may need refactor per handle)) */
	void *options;							   /* Additional initialization options passed in the initialize phase */
	set exec_path_map;						   /* Set of execution paths passed by the end user */
};

struct loader_handle_impl_type
{
	uintptr_t magic;	  /* Magic number for detecting corrupted input by the user */
	loader_impl impl;	  /* Reference to the loader which handle belongs to */
	loader_path path;	  /* File name of the module (used to index the handle) */
	loader_handle module; /* Pointer to the implementation handle, provided by the loader, it is its internal representation */
	context ctx;		  /* Contains the objects, classes and functions loaded in the handle */
	int populated;		  /* If it is populated (0), the handle context is also stored in loader context (global scope), otherwise it is private */
};

struct loader_impl_metadata_cb_iterator_type
{
	size_t iterator;
	value *values;
};

/* -- Private Methods -- */

static int loader_impl_initialize(loader_impl impl);

static dynlink loader_impl_dynlink_load(const char *path, const loader_tag tag);

static int loader_impl_dynlink_symbol(loader_impl impl, const loader_tag tag, dynlink_symbol_addr *singleton_addr_ptr);

static void loader_impl_dynlink_destroy(loader_impl impl);

static int loader_impl_create_singleton(loader_impl impl, const char *path, const loader_tag tag);

static loader_handle_impl loader_impl_load_handle(loader_impl impl, loader_handle module, const loader_path path);

static int loader_impl_handle_init(loader_impl impl, const char *path, loader_handle_impl handle_impl, void **handle_ptr, int populated);

static int loader_impl_handle_register(loader_impl impl, const char *path, loader_handle_impl handle_impl, void **handle_ptr);

static size_t loader_impl_handle_name(const loader_path path, loader_path result);

static int loader_impl_function_hook_call(context ctx, const char func_name[]);

static value loader_impl_metadata_handle_name(loader_handle_impl handle_impl);

static value loader_impl_metadata_handle_context(loader_handle_impl handle_impl);

static value loader_impl_metadata_handle(loader_handle_impl handle_impl);

static int loader_impl_metadata_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

static void loader_impl_destroy_handle(loader_handle_impl handle_impl);

static int loader_impl_destroy_type_map_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

static int loader_impl_destroy_handle_map_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

/* -- Private Member Data -- */

static const char loader_handle_impl_magic_alloc[] = "loader_handle_impl_magic_alloc";
static const char loader_handle_impl_magic_free[] = "loader_handle_impl_magic_free";

/* -- Methods -- */

dynlink loader_impl_dynlink_load(const char *path, const loader_tag tag)
{
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	const char loader_dynlink_suffix[] = "_loaderd";
#else
	const char loader_dynlink_suffix[] = "_loader";
#endif

#define LOADER_DYNLINK_NAME_SIZE \
	(sizeof(loader_dynlink_suffix) + LOADER_TAG_SIZE)

	char loader_dynlink_name[LOADER_DYNLINK_NAME_SIZE];

	strncpy(loader_dynlink_name, tag, LOADER_DYNLINK_NAME_SIZE - 1);

	strncat(loader_dynlink_name, loader_dynlink_suffix,
		LOADER_DYNLINK_NAME_SIZE - strnlen(loader_dynlink_name, LOADER_DYNLINK_NAME_SIZE - 1) - 1);

#undef LOADER_DYNLINK_NAME_SIZE

	log_write("metacall", LOG_LEVEL_DEBUG, "Loader: %s", loader_dynlink_name);

	return dynlink_load(path, loader_dynlink_name, DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);
}

int loader_impl_dynlink_symbol(loader_impl impl, const loader_tag tag, dynlink_symbol_addr *singleton_addr_ptr)
{
	const char loader_dynlink_symbol_prefix[] = DYNLINK_SYMBOL_STR("");
	const char loader_dynlink_symbol_suffix[] = "_loader_impl_interface_singleton";

#define LOADER_DYNLINK_SYMBOL_SIZE \
	(sizeof(loader_dynlink_symbol_prefix) + LOADER_TAG_SIZE + sizeof(loader_dynlink_symbol_suffix))

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

int loader_impl_create_singleton(loader_impl impl, const char *path, const loader_tag tag)
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

loader_impl loader_impl_create_proxy(void)
{
	loader_impl impl = malloc(sizeof(struct loader_impl_type));

	memset(impl, 0, sizeof(struct loader_impl_type));

	impl->init = 0; /* Do not call singleton initialize */
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
					strncpy(impl->tag, loader_host_proxy_name, LOADER_TAG_SIZE);

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

	if (execution_paths_value != NULL)
	{
		size_t size = value_type_count(execution_paths_value);
		value *execution_paths_array = value_to_array(execution_paths_value);

		if (execution_paths_array != NULL)
		{
			size_t iterator;

			for (iterator = 0; iterator < size; ++iterator)
			{
				if (execution_paths_array[iterator] != NULL)
				{
					const char *str = value_to_string(execution_paths_array[iterator]);

					if (str != NULL)
					{
						loader_path execution_path;

						strncpy(execution_path, str, LOADER_PATH_SIZE - 1);

						impl->singleton()->execution_path(impl, execution_path);
					}
				}
			}
		}
	}
}

int loader_impl_initialize(loader_impl impl)
{
	static const char loader_library_path[] = "loader_library_path";
	char configuration_key[0xFF];
	configuration config;
	value loader_library_path_value = NULL;
	const char *script_path = NULL;
	const char *library_path = NULL;
	vector paths;

	if (impl->init == 0)
	{
		return 0;
	}

	strcpy(configuration_key, impl->tag);

	strcat(configuration_key, "_loader");

	config = configuration_scope(configuration_key);

	library_path = loader_env_library_path();

	/* Check if the configuration has a custom loader_library_path, otherwise set it up */
	if (config != NULL && configuration_value(config, loader_library_path) == NULL)
	{
		loader_library_path_value = value_create_string(library_path, strlen(library_path));
		configuration_define(config, loader_library_path, loader_library_path_value);
	}

	/* Call to the loader initialize method */
	impl->data = impl->singleton()->initialize(impl, config);

	/* Undefine the library path field from config */
	if (config != NULL && loader_library_path_value != NULL)
	{
		configuration_undefine(config, loader_library_path);
		value_type_destroy(loader_library_path_value);
	}

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

	/* Load the library path as execution path */
	if (library_path != NULL)
	{
		if (loader_impl_execution_path(impl, library_path) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Error when loading path %s", library_path);
		}
	}

	script_path = loader_env_script_path();

	/* Note: I think we should not allow multiple paths for LOADER_SCRIPT_PATH.
	* This provides name collision with the handles because handle names are defined by its
	* relative path if they are inside LOADER_SCRIPT_PATH or by its absolute path if they are outside.
	* If there's multiple LOADER_SCRIPT_PATH there can be collisions with relative handle names.
	* For now I am going to disable it and we will review it in the future.
	* An alternative to support multiple execution paths is pretty straightforward, just call
	* to the execution path API and register the paths you want for each loader, so you have
	* total control for multiple search paths.
	*/

	if (script_path != NULL)
	{
		if (loader_impl_execution_path(impl, script_path) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Error when loading path %s", script_path);
		}
	}

#if 0
	/* Split multiple paths */
	char path_copy[LOADER_PATH_SIZE + 1];
	strncpy(path_copy, script_path, LOADER_PATH_SIZE);

	char *split_path = strrchr(path_copy, LOADER_PATH_DELIMITER);
	while (split_path != NULL)
	{
		*split_path = '\0'; /* Replace the delimiter with a terminator */
		split_path++;		/* Move a char to the right to avoid the delimiter */

		if (loader_impl_execution_path(impl, split_path) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Error when loading path %s", split_path);
		}

		split_path = strrchr(path_copy, LOADER_PATH_DELIMITER);
	}

	/* Add the path without delimiter */
	if (loader_impl_execution_path(impl, path_copy) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Error when loading path %s", path_copy);
	}
#endif

	paths = set_get(impl->exec_path_map, (const set_key)impl->tag);

	/* Load all execution paths */
	if (paths != NULL)
	{
		size_t iterator, size = vector_size(paths);

		for (iterator = 0; iterator < size; ++iterator)
		{
			char *path = vector_at(paths, iterator);

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

loader_impl loader_impl_create(const char *path, const loader_tag tag)
{
	if (tag != NULL)
	{
		loader_impl impl = malloc(sizeof(struct loader_impl_type));

		if (impl == NULL)
		{
			return NULL;
		}

		impl->init = 1;
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
						strncpy(impl->tag, tag, LOADER_TAG_SIZE - 1);

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

value loader_impl_get_value(loader_impl impl, const char *name)
{
	context ctx = loader_impl_context(impl);
	scope sp = context_scope(ctx);
	return scope_get(sp, name);
}

loader_impl_interface loader_impl_symbol(loader_impl impl)
{
	if (impl != NULL && impl->singleton != NULL)
	{
		return impl->singleton();
	}

	return NULL;
}

loader_tag *loader_impl_tag(loader_impl impl)
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

type loader_impl_type(loader_impl impl, const char *name)
{
	if (impl != NULL && impl->type_info_map != NULL && name != NULL)
	{
		return (type)set_get(impl->type_info_map, (const set_key)name);
	}

	return NULL;
}

int loader_impl_type_define(loader_impl impl, const char *name, type t)
{
	if (impl != NULL && impl->type_info_map != NULL && name != NULL)
	{
		return set_insert(impl->type_info_map, (const set_key)name, (set_value)t);
	}

	return 1;
}

loader_handle_impl loader_impl_load_handle(loader_impl impl, loader_handle module, const loader_path path)
{
	loader_handle_impl handle_impl = malloc(sizeof(struct loader_handle_impl_type));

	if (handle_impl != NULL)
	{
		handle_impl->impl = impl;
		strncpy(handle_impl->path, path, LOADER_PATH_SIZE);
		handle_impl->module = module;
		handle_impl->ctx = context_create(handle_impl->path);

		if (handle_impl->ctx == NULL)
		{
			handle_impl->magic = (uintptr_t)loader_handle_impl_magic_free;
			free(handle_impl);
		}

		handle_impl->magic = (uintptr_t)loader_handle_impl_magic_alloc;
		return handle_impl;
	}

	return NULL;
}

void loader_impl_destroy_handle(loader_handle_impl handle_impl)
{
	if (handle_impl != NULL)
	{
		static const char func_fini_name[] = LOADER_IMPL_FUNCTION_FINI;

		loader_impl_interface interface_impl = loader_impl_symbol(handle_impl->impl);

		if (handle_impl->impl->init == 0)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Destroying handle %s", handle_impl->path);

			if (loader_impl_function_hook_call(handle_impl->ctx, func_fini_name) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Error when calling destructor from handle impl: %p (%s)", (void *)handle_impl, func_fini_name);
			}

			if (interface_impl->clear(handle_impl->impl, handle_impl->module) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Error when clearing handle impl: %p", (void *)handle_impl);
			}
		}

		if (handle_impl->populated == 0)
		{
			context_remove(handle_impl->impl->ctx, handle_impl->ctx);
		}

		context_destroy(handle_impl->ctx);
		handle_impl->magic = (uintptr_t)loader_handle_impl_magic_free;

		free(handle_impl);
	}
}

int loader_impl_execution_path(loader_impl impl, const loader_path path)
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
				paths = vector_create(sizeof(char) * LOADER_PATH_SIZE);

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
		void *null_args[1] = { NULL };

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

int loader_impl_handle_init(loader_impl impl, const char *path, loader_handle_impl handle_impl, void **handle_ptr, int populated)
{
	static const char func_init_name[] = LOADER_IMPL_FUNCTION_INIT;

	int result = loader_impl_function_hook_call(impl->ctx, func_init_name);

	handle_impl->populated = populated;

	if (result != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Error when calling to init hook function (" LOADER_IMPL_FUNCTION_INIT ") of handle: %s", path);
	}

	if (handle_ptr != NULL)
	{
		*handle_ptr = handle_impl;
	}

	return result;
}

int loader_impl_handle_register(loader_impl impl, const char *path, loader_handle_impl handle_impl, void **handle_ptr)
{
	if (handle_ptr == NULL)
	{
		char *duplicated_key = NULL;

		if (context_contains(impl->ctx, handle_impl->ctx, &duplicated_key) == 0)
		{
			/* TODO: This still does not protect duplicated names between different loaders global scope */
			log_write("metacall", LOG_LEVEL_ERROR, "Duplicated symbol found named '%s' already defined in the global scope by handle: %s", duplicated_key, path);
		}
		else if (context_append(impl->ctx, handle_impl->ctx) == 0)
		{
			return loader_impl_handle_init(impl, path, handle_impl, handle_ptr, 0);
		}
	}
	else
	{
		return loader_impl_handle_init(impl, path, handle_impl, handle_ptr, 1);
	}

	return 1;
}

size_t loader_impl_handle_name(const loader_path path, loader_path result)
{
	const char *script_path = loader_env_script_path();
	size_t script_path_size = strlen(script_path) + 1;
	size_t path_size = strnlen(path, LOADER_PATH_SIZE) + 1;

	if (portability_path_is_subpath(script_path, script_path_size, path, path_size))
	{
		return portability_path_get_relative(script_path, script_path_size, path, path_size, result, LOADER_PATH_SIZE) - 1;
	}
	else
	{
		strncpy(result, path, LOADER_PATH_SIZE - 1);

		return strnlen(result, LOADER_PATH_SIZE);
	}
}

int loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size, void **handle_ptr)
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
			loader_path path;

			if (loader_impl_initialize(impl) != 0)
			{
				return 1;
			}

			if (loader_impl_handle_name(paths[0], path) > 1 && loader_impl_get_handle(impl, path) != NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Load from file handle failed, handle with name %s already loaded", path);

				return 1;
			}

			handle = interface_impl->load_from_file(impl, paths, size);

			log_write("metacall", LOG_LEVEL_DEBUG, "Loader interface: %p - Loader handle: %p", (void *)interface_impl, (void *)handle);

			if (handle != NULL)
			{
				loader_handle_impl handle_impl = loader_impl_load_handle(impl, handle, path);

				log_write("metacall", LOG_LEVEL_DEBUG, "Loader handle impl: %p", (void *)handle_impl);

				if (handle_impl != NULL)
				{
					handle_impl->populated = 1;

					if (set_insert(impl->handle_impl_map, handle_impl->path, handle_impl) == 0)
					{
						if (interface_impl->discover(impl, handle_impl->module, handle_impl->ctx) == 0)
						{
							if (loader_impl_handle_register(impl, path, handle_impl, handle_ptr) == 0)
							{
								return 0;
							}
						}

						set_remove(impl->handle_impl_map, handle_impl->path);
					}

					log_write("metacall", LOG_LEVEL_ERROR, "Error when loading handle: %s", path);

					loader_impl_destroy_handle(handle_impl);
				}
			}
		}
	}

	return 1;
}

int loader_impl_load_from_memory_name(loader_impl impl, loader_name name, const char *buffer, size_t size)
{
	/* TODO: Improve name with time or uuid */
	static const char format[] = "%p-%p-%" PRIuS "-%u";

	hash h = hash_callback_str((const hash_key)buffer);

	size_t length = snprintf(NULL, 0, format, (const void *)impl, (const void *)buffer, size, (unsigned int)h);

	if (length > 0 && length < LOADER_NAME_SIZE)
	{
		size_t written = snprintf(name, length + 1, format, (const void *)impl, (const void *)buffer, size, (unsigned int)h);

		if (written == length)
		{
			return 0;
		}
	}

	return 1;
}

int loader_impl_load_from_memory(loader_impl impl, const char *buffer, size_t size, void **handle_ptr)
{
	if (impl != NULL && buffer != NULL && size > 0)
	{
		loader_impl_interface interface_impl = loader_impl_symbol(impl);

		log_write("metacall", LOG_LEVEL_DEBUG, "Loading from memory %.10s...", buffer);

		if (interface_impl != NULL)
		{
			loader_name name;

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

			log_write("metacall", LOG_LEVEL_DEBUG, "Loader interface: %p - Loader handle: %p", (void *)interface_impl, (void *)handle);

			if (handle != NULL)
			{
				loader_handle_impl handle_impl = loader_impl_load_handle(impl, handle, name);

				if (handle_impl != NULL)
				{
					handle_impl->populated = 1;

					if (set_insert(impl->handle_impl_map, handle_impl->path, handle_impl) == 0)
					{
						if (interface_impl->discover(impl, handle_impl->module, handle_impl->ctx) == 0)
						{
							if (loader_impl_handle_register(impl, name, handle_impl, handle_ptr) == 0)
							{
								return 0;
							}
						}

						set_remove(impl->handle_impl_map, handle_impl->path);
					}

					log_write("metacall", LOG_LEVEL_ERROR, "Error when loading handle: %s", name);

					loader_impl_destroy_handle(handle_impl);
				}
			}
		}
	}

	return 1;
}

int loader_impl_load_from_package(loader_impl impl, const loader_path path, void **handle_ptr)
{
	if (impl != NULL)
	{
		loader_impl_interface interface_impl = loader_impl_symbol(impl);

		loader_path subpath;

		if (interface_impl != NULL && loader_impl_handle_name(path, subpath) > 1)
		{
			loader_handle handle;

			if (loader_impl_initialize(impl) != 0)
			{
				return 1;
			}

			if (loader_impl_get_handle(impl, subpath) != NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Load from package handle failed, handle with name %s already loaded", subpath);

				return 1;
			}

			handle = interface_impl->load_from_package(impl, path);

			log_write("metacall", LOG_LEVEL_DEBUG, "Loader interface: %p - Loader handle: %p", (void *)interface_impl, (void *)handle);

			if (handle != NULL)
			{
				loader_handle_impl handle_impl = loader_impl_load_handle(impl, handle, subpath);

				if (handle_impl != NULL)
				{
					handle_impl->populated = 1;

					if (set_insert(impl->handle_impl_map, handle_impl->path, handle_impl) == 0)
					{
						if (interface_impl->discover(impl, handle_impl->module, handle_impl->ctx) == 0)
						{
							if (loader_impl_handle_register(impl, subpath, handle_impl, handle_ptr) == 0)
							{
								return 0;
							}
						}

						set_remove(impl->handle_impl_map, handle_impl->path);
					}

					log_write("metacall", LOG_LEVEL_ERROR, "Error when loading handle: %s", subpath);

					loader_impl_destroy_handle(handle_impl);
				}
			}
		}
	}

	return 1;
}

void *loader_impl_get_handle(loader_impl impl, const char *name)
{
	if (impl != NULL && name != NULL)
	{
		return (void *)set_get(impl->handle_impl_map, (set_key)name);
	}

	return NULL;
}

void loader_impl_set_options(loader_impl impl, void *options)
{
	if (impl != NULL && options != NULL)
	{
		impl->options = options;
	}
}

void *loader_impl_get_options(loader_impl impl)
{
	if (impl != NULL)
	{
		return impl->options;
	}

	return NULL;
}

const char *loader_impl_handle_id(void *handle)
{
	loader_handle_impl handle_impl = handle;

	return handle_impl->path;
}

value loader_impl_handle_export(void *handle)
{
	loader_handle_impl handle_impl = handle;

	return scope_export(context_scope(handle_impl->ctx));
}

context loader_impl_handle_context(void *handle)
{
	loader_handle_impl handle_impl = handle;

	return handle_impl->ctx;
}

int loader_impl_handle_validate(void *handle)
{
	loader_handle_impl handle_impl = handle;

	return !(handle_impl != NULL && handle_impl->magic == (uintptr_t)loader_handle_impl_magic_alloc);
}

value loader_impl_metadata_handle_name(loader_handle_impl handle_impl)
{
	static const char name[] = "name";

	value *v_ptr, v = value_create_array(NULL, 2);

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

	v_ptr[1] = value_create_string(handle_impl->path, strlen(handle_impl->path));

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

	value *v_ptr, v = value_create_array(NULL, 2);

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
	value *v_ptr, v = value_create_map(NULL, 2);

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

int loader_impl_clear(void *handle)
{
	if (handle != NULL)
	{
		loader_handle_impl handle_impl = handle;

		loader_impl impl = handle_impl->impl;

		int result = !(set_remove(impl->handle_impl_map, (set_key)(handle_impl->path)) == handle_impl);

		loader_impl_destroy_handle(handle_impl);

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

		loader_impl_destroy_handle(handle_impl);

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

void loader_impl_destroy_objects(loader_impl impl)
{
	/* This iterates through all functions, classes objects and types,
	* it is necessary to be executed on demand because those can have
	* implementations in the loader implementation which need to be GCed
	* or freed properly before the runtime goes down but after the
	* destroy has been issued, so while it is destroying, we can still
	* retrieve the data for introspection or for whatever we need
	*/
	if (impl != NULL)
	{
		set_iterate(impl->handle_impl_map, &loader_impl_destroy_handle_map_cb_iterate, NULL);

		set_destroy(impl->handle_impl_map);

		set_iterate(impl->type_info_map, &loader_impl_destroy_type_map_cb_iterate, NULL);

		set_destroy(impl->type_info_map);
	}
}

void loader_impl_destroy(loader_impl impl)
{
	if (impl != NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "Destroy loader implementation %s", impl->tag);

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

		context_destroy(impl->ctx);

		loader_impl_dynlink_destroy(impl);

		free(impl);
	}
}
