/*
 *	Loader Library by Parra Studios
 *	A library for loading executable code at run-time into a process.
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

#include <loader/loader_impl.h>
#include <loader/loader_manager_impl.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_type.h>

#include <adt/adt_hash.h>
#include <adt/adt_set.h>
#include <adt/adt_vector.h>

#include <format/format_print.h>

#include <log/log.h>

#include <configuration/configuration.h>

#include <stdlib.h>
#include <string.h>

/* -- Macros -- */

#define loader_iface(l) \
	plugin_iface_type(l, loader_impl_interface)

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

struct loader_impl_handle_register_cb_iterator_type;

/* -- Type Definitions -- */

typedef struct loader_handle_impl_type *loader_handle_impl;

typedef struct loader_impl_metadata_cb_iterator_type *loader_impl_metadata_cb_iterator;

typedef struct loader_impl_handle_register_cb_iterator_type *loader_impl_handle_register_cb_iterator;

/* -- Member Data -- */

struct loader_impl_type
{
	plugin p;					   /* Plugin instance to which loader belongs to */
	int init;					   /* Flag for checking if the loader is initialized */
	set handle_impl_path_map;	   /* Indexes handles by path */
	set handle_impl_map;		   /* Indexes handles from loaders to handle impl (loader_handle -> loader_handle_impl) */
	vector handle_impl_init_order; /* Stores the order of handle initialization, so it can be destroyed in LIFO manner, for avoiding memory bugs when destroying them */
	loader_impl_data data;		   /* Derived metadata provided by the loader, usually contains the data of the VM, Interpreter or JIT */
	context ctx;				   /* Contains the objects, classes and functions loaded in the global scope of each loader */
	set type_info_map;			   /* Stores a set indexed by type name of all of the types existing in the loader (global scope (TODO: may need refactor per handle)) */
	void *options;				   /* Additional initialization options passed in the initialize phase */
	set exec_path_map;			   /* Set of execution paths passed by the end user */
};

struct loader_handle_impl_type
{
	uintptr_t magic;			 /* Magic number for detecting corrupted input by the user */
	loader_impl impl;			 /* Reference to the loader which handle belongs to */
	loader_impl_interface iface; /* Reference to the loader interface which handle belongs to */
	loader_path path;			 /* File name of the module (used to index the handle) */
	loader_handle module;		 /* Pointer to the implementation handle, provided by the loader, it is its internal representation */
	context ctx;				 /* Contains the objects, classes and functions loaded in the handle */
	int populated;				 /* If it is populated (0), the handle context is also stored in loader context (global scope), otherwise it is private */
	vector populated_handles;	 /* Vector containing all the references to which this handle has been populated into, it is necessary for detach the symbols when destroying (used in load_from_* when passing an input parameter) */
};

struct loader_impl_handle_register_cb_iterator_type
{
	context handle_ctx;
	char *duplicated_key;
};

struct loader_impl_metadata_cb_iterator_type
{
	size_t iterator;
	value *values;
};

/* -- Private Methods -- */

static loader_impl loader_impl_allocate(const loader_tag tag);

static configuration loader_impl_initialize_configuration(plugin p);

static int loader_impl_initialize_registered(plugin_manager manager, plugin p);

static int loader_impl_initialize(plugin_manager manager, plugin p, loader_impl impl);

static loader_handle_impl loader_impl_load_handle(loader_impl impl, loader_impl_interface iface, loader_handle module, const loader_path path);

static int loader_impl_handle_init(loader_impl impl, const char *path, loader_handle_impl handle_impl, void **handle_ptr, int populated);

static int loader_impl_handle_register_cb_iterate(plugin_manager manager, plugin p, void *data);

static int loader_impl_handle_register(plugin_manager manager, loader_impl impl, const char *path, loader_handle_impl handle_impl, void **handle_ptr);

static size_t loader_impl_handle_name(plugin_manager manager, const loader_path path, loader_path result);

static int loader_impl_function_hook_call(context ctx, const char func_name[]);

static value loader_impl_metadata_handle_name(loader_handle_impl handle_impl);

static value loader_impl_metadata_handle_context(loader_handle_impl handle_impl);

static value loader_impl_metadata_handle(loader_handle_impl handle_impl);

static int loader_impl_metadata_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

static void loader_impl_destroy_handle(loader_handle_impl handle_impl);

static int loader_impl_destroy_type_map_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

/* -- Private Member Data -- */

static const char loader_handle_impl_magic_alloc[] = "loader_handle_impl_magic_alloc";
static const char loader_handle_impl_magic_free[] = "loader_handle_impl_magic_free";

/* -- Methods -- */

loader_impl loader_impl_allocate(const loader_tag tag)
{
	loader_impl impl = malloc(sizeof(struct loader_impl_type));

	if (impl == NULL)
	{
		goto alloc_error;
	}

	memset(impl, 0, sizeof(struct loader_impl_type));

	impl->handle_impl_path_map = set_create(&hash_callback_str, &comparable_callback_str);

	if (impl->handle_impl_path_map == NULL)
	{
		goto alloc_handle_impl_path_map_error;
	}

	impl->handle_impl_map = set_create(&hash_callback_ptr, &comparable_callback_ptr);

	if (impl->handle_impl_map == NULL)
	{
		goto alloc_handle_impl_map_error;
	}

	impl->handle_impl_init_order = vector_create_type(loader_handle_impl);

	if (impl->handle_impl_init_order == NULL)
	{
		goto alloc_handle_impl_init_order_error;
	}

	impl->type_info_map = set_create(&hash_callback_str, &comparable_callback_str);

	if (impl->type_info_map == NULL)
	{
		goto alloc_type_info_map_error;
	}

	impl->ctx = context_create(tag);

	if (impl->ctx == NULL)
	{
		goto alloc_ctx_error;
	}

	impl->exec_path_map = set_create(&hash_callback_str, &comparable_callback_str);

	if (impl->exec_path_map == NULL)
	{
		goto alloc_exec_path_map_error;
	}

	return impl;

alloc_exec_path_map_error:
	context_destroy(impl->ctx);
alloc_ctx_error:
	set_destroy(impl->type_info_map);
alloc_type_info_map_error:
	vector_destroy(impl->handle_impl_init_order);
alloc_handle_impl_init_order_error:
	set_destroy(impl->handle_impl_map);
alloc_handle_impl_map_error:
	set_destroy(impl->handle_impl_path_map);
alloc_handle_impl_path_map_error:
	free(impl);
alloc_error:
	return NULL;
}

loader_impl loader_impl_create_host(const loader_tag tag)
{
	loader_impl impl = loader_impl_allocate(tag);

	if (impl == NULL)
	{
		return NULL;
	}

	impl->init = 0; /* Do not call singleton initialize */
	impl->options = NULL;

	return impl;
}

void loader_impl_attach(loader_impl impl, plugin p)
{
	if (impl != NULL && p != NULL)
	{
		impl->p = p;
	}
}

plugin loader_impl_plugin(loader_impl impl)
{
	if (impl != NULL)
	{
		return impl->p;
	}

	return NULL;
}

void loader_impl_configuration(loader_impl_interface iface, loader_impl impl, configuration config)
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
					size_t str_size = value_type_size(execution_paths_array[iterator]);

					if (str != NULL)
					{
						loader_path execution_path;

						strncpy(execution_path, str, str_size > LOADER_PATH_SIZE ? LOADER_PATH_SIZE : str_size);

						iface->execution_path(impl, execution_path);
					}
				}
			}
		}
	}
}

configuration loader_impl_initialize_configuration(plugin p)
{
	static const char configuration_key_suffix[] = "_loader";
#define CONFIGURATION_KEY_SIZE ((size_t)sizeof(configuration_key_suffix) + LOADER_TAG_SIZE - 1)
	char configuration_key[CONFIGURATION_KEY_SIZE];

	/* Retrieve the configuration key: <tag>_loader */
	size_t tag_size = strnlen(plugin_name(p), LOADER_TAG_SIZE) + 1;

	strncpy(configuration_key, plugin_name(p), tag_size);

	strncat(configuration_key, configuration_key_suffix, CONFIGURATION_KEY_SIZE - tag_size);
#undef CONFIGURATION_KEY_SIZE

	return configuration_scope(configuration_key);
}

int loader_impl_initialize_registered(plugin_manager manager, plugin p)
{
	loader_manager_impl manager_impl = plugin_manager_impl_type(manager, loader_manager_impl);
	size_t iterator, size = vector_size(manager_impl->initialization_order);

	/* Check if the plugin has been properly registered into initialization order list */
	for (iterator = 0; iterator < size; ++iterator)
	{
		loader_initialization_order order = vector_at(manager_impl->initialization_order, iterator);

		if (order->p == p)
		{
			return 0;
		}
	}

	return 1;
}

int loader_impl_initialize(plugin_manager manager, plugin p, loader_impl impl)
{
	static const char loader_library_path[] = "loader_library_path";
	configuration config;
	value loader_library_path_value = NULL;
	char *library_path = NULL;
	vector script_paths, paths;

	if (impl->init == 0)
	{
		return 0;
	}

	/* Get the configuration of the loader */
	config = loader_impl_initialize_configuration(p);

	/* Retrieve the library path */
	library_path = plugin_manager_library_path(manager);

	/* The library path priority order is the following:
	* 1) In the configuration <tag>_loader.json, the flag loader_library_path.
	* 2) The environment variable LOADER_LIBRARY_PATH.
	* 3) The directory of the path of (lib)metacall.{so,dll,dylib,...}
	* 4) Default install paths defined at compile time.
	*/

	/* Check if the configuration has a custom loader_library_path, otherwise set it up */
	if (config != NULL && configuration_value(config, loader_library_path) == NULL)
	{
		loader_library_path_value = value_create_string(library_path, strnlen(library_path, LOADER_PATH_SIZE));
		configuration_define(config, loader_library_path, loader_library_path_value);
	}

	/* Call to the loader initialize method */
	impl->data = loader_iface(p)->initialize(impl, config);

	/* Undefine the library path field from config */
	if (config != NULL && loader_library_path_value != NULL)
	{
		configuration_undefine(config, loader_library_path);
		value_type_destroy(loader_library_path_value);
	}

	/* Verify that it the loader returned valid data on initialization */
	if (impl->data == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader (%s) returned NULL value on the initialization", plugin_name(p));
		return 1;
	}

	/* Verify that the loader has been registered into the initialization list */
	if (loader_impl_initialize_registered(manager, p) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader (%s) was not registered in the initialization list, "
											   "in order to solve this problem it must call to the function 'loader_initialization_register' "
											   "at the end of the initialization (i.e %s_loader_impl_initialize)",
			plugin_name(p), plugin_name(p));
		return 1;
	}

	impl->init = 0;

	if (config != NULL)
	{
		loader_impl_configuration(loader_iface(p), impl, config);
	}

	/* The scripts path priority order is the following:
	* 1) The library path.
	* 2) The delimiter based paths defined in LOADER_SCRIPT_PATH.
	* 3) The execution paths defined before the initialization of a loader (for example, all metacall_execution_path(...) called before metacall_load_from_file(...)).
	*/

	/* Load the library path as execution path */
	loader_library_path_value = configuration_value(config, loader_library_path);

	if (loader_library_path_value != NULL)
	{
		library_path = value_to_string(loader_library_path_value);
	}

	if (loader_impl_execution_path(p, impl, library_path) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Error when loading path %s", library_path);
	}

	script_paths = plugin_manager_impl_type(manager, loader_manager_impl)->script_paths;

	/* Load the list of script paths (this allows to load multiple paths with a delimiter) */
	if (script_paths != NULL)
	{
		size_t iterator, size = vector_size(script_paths);

		for (iterator = 0; iterator < size; ++iterator)
		{
			char *path = vector_at_type(script_paths, iterator, char *);

			if (loader_impl_execution_path(p, impl, path) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Loader (%s) failed to define execution path: %s", plugin_name(p), path);
			}
		}
	}

	paths = set_get(impl->exec_path_map, (const set_key)plugin_name(p));

	/* Load all execution paths (those are the delayed loading for the paths that has been defined before initializing the loader) */
	if (paths != NULL)
	{
		size_t iterator, size = vector_size(paths);

		for (iterator = 0; iterator < size; ++iterator)
		{
			char *path = vector_at(paths, iterator);

			if (loader_impl_execution_path(p, impl, path) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Loader (%s) failed to load path: %s", plugin_name(p), path);
			}
		}
	}

	return 0;
}

int loader_impl_is_initialized(loader_impl impl)
{
	return impl->init;
}

loader_impl loader_impl_create(const loader_tag tag)
{
	loader_impl impl = loader_impl_allocate(tag);

	if (impl == NULL)
	{
		return NULL;
	}

	impl->init = 1;
	impl->options = NULL;

	return impl;
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

loader_handle_impl loader_impl_load_handle(loader_impl impl, loader_impl_interface iface, loader_handle module, const loader_path path)
{
	loader_handle_impl handle_impl = malloc(sizeof(struct loader_handle_impl_type));

	if (handle_impl == NULL)
	{
		goto alloc_loader_handle_impl_error;
	}

	handle_impl->impl = impl;
	handle_impl->iface = iface;
	strncpy(handle_impl->path, path, LOADER_PATH_SIZE);
	handle_impl->module = module;
	handle_impl->ctx = context_create(handle_impl->path);

	if (handle_impl->ctx == NULL)
	{
		goto alloc_context_error;
	}

	handle_impl->populated_handles = vector_create_type(loader_handle_impl);

	if (handle_impl->populated_handles == NULL)
	{
		goto alloc_populated_handles_error;
	}

	handle_impl->magic = (uintptr_t)loader_handle_impl_magic_alloc;

	return handle_impl;

alloc_populated_handles_error:
	context_destroy(handle_impl->ctx);
alloc_context_error:
	handle_impl->magic = (uintptr_t)loader_handle_impl_magic_free;
	free(handle_impl);
alloc_loader_handle_impl_error:
	return NULL;
}

void loader_impl_destroy_handle(loader_handle_impl handle_impl)
{
	if (handle_impl != NULL)
	{
		static const char func_fini_name[] = LOADER_IMPL_FUNCTION_FINI;
		size_t iterator;

		if (handle_impl->impl->init == 0)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Destroying handle %s", handle_impl->path);

			if (loader_impl_function_hook_call(handle_impl->ctx, func_fini_name) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Error when calling destructor from handle impl: %p (%s)", (void *)handle_impl, func_fini_name);
			}

			if (handle_impl->iface->clear(handle_impl->impl, handle_impl->module) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Error when clearing handle impl: %p", (void *)handle_impl);
			}
		}

		if (handle_impl->populated == 0)
		{
			context_remove(handle_impl->impl->ctx, handle_impl->ctx);
		}

		for (iterator = 0; iterator < vector_size(handle_impl->populated_handles); ++iterator)
		{
			loader_handle_impl populated_handle_impl = vector_at_type(handle_impl->populated_handles, iterator, loader_handle_impl);

			if (populated_handle_impl->populated == 0)
			{
				context_remove(populated_handle_impl->impl->ctx, handle_impl->ctx);
			}

			context_remove(populated_handle_impl->ctx, handle_impl->ctx);
		}

		context_destroy(handle_impl->ctx);
		vector_destroy(handle_impl->populated_handles);
		handle_impl->magic = (uintptr_t)loader_handle_impl_magic_free;

		free(handle_impl);
	}
}

int loader_impl_execution_path(plugin p, loader_impl impl, const loader_path path)
{
	if (impl != NULL)
	{
		if (impl->init == 0)
		{
			/* If loader is initialized, load the execution path */
			loader_impl_interface iface = loader_iface(p);

			if (iface != NULL)
			{
				return iface->execution_path(impl, path);
			}
		}
		else
		{
			/* If loader is not initialized, store the execution path for later use */
			vector paths = set_get(impl->exec_path_map, (const set_key)plugin_name(p));

			if (paths == NULL)
			{
				paths = vector_create(sizeof(char) * LOADER_PATH_SIZE);

				if (paths == NULL)
				{
					return 1;
				}

				if (set_insert(impl->exec_path_map, (set_key)plugin_name(p), paths) != 0)
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

int loader_impl_handle_register_cb_iterate(plugin_manager manager, plugin p, void *data)
{
	loader_impl impl = plugin_impl_type(p, loader_impl);
	loader_impl_handle_register_cb_iterator iterator = (loader_impl_handle_register_cb_iterator)data;

	(void)manager;

	return (context_contains(impl->ctx, iterator->handle_ctx, &iterator->duplicated_key) == 0);
}

int loader_impl_handle_register(plugin_manager manager, loader_impl impl, const char *path, loader_handle_impl handle_impl, void **handle_ptr)
{
	/* If there's no handle input/output pointer passed as input parameter, then propagate the handle symbols to the loader context */
	if (handle_ptr == NULL)
	{
		/* This case handles the global scope (shared scope between all loaders, there is no out reference to a handle) */
		struct loader_impl_handle_register_cb_iterator_type iterator;

		iterator.handle_ctx = handle_impl->ctx;
		iterator.duplicated_key = NULL;

		/* This checks if there are duplicated keys between all loaders and the current handle context */
		plugin_manager_iterate(manager, &loader_impl_handle_register_cb_iterate, &iterator);

		if (iterator.duplicated_key != NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Duplicated symbol found named '%s' already defined in the global scope by handle: %s", iterator.duplicated_key, path);
			return 1;
		}
		else if (context_append(impl->ctx, handle_impl->ctx) == 0)
		{
			return loader_impl_handle_init(impl, path, handle_impl, handle_ptr, 0);
		}
	}
	else
	{
		/* Otherwise, if there's a handle pointer and it is different from NULL, it means we are passing a handle as input parameter, so propagate symbols to this handle */
		if (*handle_ptr != NULL)
		{
			loader_handle_impl target_handle = (loader_handle_impl)*handle_ptr;
			char *duplicated_key;

			if (context_contains(handle_impl->ctx, target_handle->ctx, &duplicated_key) == 0 && duplicated_key != NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Duplicated symbol found named '%s' already defined in the handle scope by handle: %s", duplicated_key, path);
				return 1;
			}
			else if (context_append(target_handle->ctx, handle_impl->ctx) == 0)
			{
				vector_push_back_var(handle_impl->populated_handles, target_handle);

				return loader_impl_handle_init(impl, path, handle_impl, NULL, 1);
			}
		}
		else
		{
			/* Otherwise, initialize the handle and do not propagate the symbols, keep it private to the handle instance */
			return loader_impl_handle_init(impl, path, handle_impl, handle_ptr, 1);
		}
	}

	return 1;
}

size_t loader_impl_handle_name(plugin_manager manager, const loader_path path, loader_path result)
{
	vector script_paths = plugin_manager_impl_type(manager, loader_manager_impl)->script_paths;

	if (script_paths != NULL)
	{
		size_t iterator, size = vector_size(script_paths);

		/* TODO: Should scripts_path be sorted in order to prevent name collisions? */
		for (iterator = 0; iterator < size; ++iterator)
		{
			char *script_path = vector_at_type(script_paths, iterator, char *);
			size_t script_path_size = strnlen(script_path, LOADER_PATH_SIZE) + 1;
			size_t path_size = strnlen(path, LOADER_PATH_SIZE) + 1;

			if (portability_path_is_subpath(script_path, script_path_size, path, path_size))
			{
				return portability_path_get_relative(script_path, script_path_size, path, path_size, result, LOADER_PATH_SIZE) - 1;
			}
		}
	}

	size_t length = strnlen(path, LOADER_PATH_SIZE);

	memcpy(result, path, length + 1);

	return length;
}

int loader_impl_load_from_file(plugin_manager manager, plugin p, loader_impl impl, const loader_path paths[], size_t size, void **handle_ptr)
{
	if (impl != NULL)
	{
		loader_impl_interface iface = loader_iface(p);

		size_t iterator;

		for (iterator = 0; iterator < size; ++iterator)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Loading %s", paths[iterator]);
		}

		if (iface != NULL)
		{
			loader_handle handle;
			loader_path path;

			if (loader_impl_initialize(manager, p, impl) != 0)
			{
				return 1;
			}

			if (loader_impl_handle_name(manager, paths[0], path) > 1 && loader_impl_get_handle(impl, path) != NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Load from file handle failed, handle with name %s already loaded", path);

				return 1;
			}

			handle = iface->load_from_file(impl, paths, size);

			log_write("metacall", LOG_LEVEL_DEBUG, "Loader interface: %p - Loader handle: %p", (void *)iface, (void *)handle);

			if (handle != NULL)
			{
				loader_handle_impl handle_impl = loader_impl_load_handle(impl, iface, handle, path);

				log_write("metacall", LOG_LEVEL_DEBUG, "Loader handle impl: %p", (void *)handle_impl);

				if (handle_impl != NULL)
				{
					handle_impl->populated = 1;

					if (set_insert(impl->handle_impl_path_map, handle_impl->path, handle_impl) == 0)
					{
						if (set_insert(impl->handle_impl_map, handle_impl->module, handle_impl) == 0)
						{
							if (iface->discover(impl, handle_impl->module, handle_impl->ctx) == 0)
							{
								if (loader_impl_handle_register(manager, impl, path, handle_impl, handle_ptr) == 0)
								{
									vector_push_back_var(impl->handle_impl_init_order, handle_impl);

									return 0;
								}
							}

							set_remove(impl->handle_impl_map, handle_impl->module);
						}

						set_remove(impl->handle_impl_path_map, handle_impl->path);
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

int loader_impl_load_from_memory(plugin_manager manager, plugin p, loader_impl impl, const char *buffer, size_t size, void **handle_ptr)
{
	if (impl != NULL && buffer != NULL && size > 0)
	{
		loader_impl_interface iface = loader_iface(p);

		log_write("metacall", LOG_LEVEL_DEBUG, "Loading from memory %.10s...", buffer);

		if (iface != NULL)
		{
			loader_name name;

			loader_handle handle = NULL;

			if (loader_impl_initialize(manager, p, impl) != 0)
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

			handle = iface->load_from_memory(impl, name, buffer, size);

			log_write("metacall", LOG_LEVEL_DEBUG, "Loader interface: %p - Loader handle: %p", (void *)iface, (void *)handle);

			if (handle != NULL)
			{
				loader_handle_impl handle_impl = loader_impl_load_handle(impl, iface, handle, name);

				if (handle_impl != NULL)
				{
					handle_impl->populated = 1;

					if (set_insert(impl->handle_impl_path_map, handle_impl->path, handle_impl) == 0)
					{
						if (set_insert(impl->handle_impl_map, handle_impl->module, handle_impl) == 0)
						{
							if (iface->discover(impl, handle_impl->module, handle_impl->ctx) == 0)
							{
								if (loader_impl_handle_register(manager, impl, name, handle_impl, handle_ptr) == 0)
								{
									vector_push_back_var(impl->handle_impl_init_order, handle_impl);

									return 0;
								}
							}

							set_remove(impl->handle_impl_map, handle_impl->module);
						}

						set_remove(impl->handle_impl_path_map, handle_impl->path);
					}

					log_write("metacall", LOG_LEVEL_ERROR, "Error when loading handle: %s", name);

					loader_impl_destroy_handle(handle_impl);
				}
			}
		}
	}

	return 1;
}

int loader_impl_load_from_package(plugin_manager manager, plugin p, loader_impl impl, const loader_path path, void **handle_ptr)
{
	if (impl != NULL)
	{
		loader_impl_interface iface = loader_iface(p);

		loader_path subpath;

		if (iface != NULL && loader_impl_handle_name(manager, path, subpath) > 1)
		{
			loader_handle handle;

			if (loader_impl_initialize(manager, p, impl) != 0)
			{
				return 1;
			}

			if (loader_impl_get_handle(impl, subpath) != NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Load from package handle failed, handle with name %s already loaded", subpath);

				return 1;
			}

			handle = iface->load_from_package(impl, path);

			log_write("metacall", LOG_LEVEL_DEBUG, "Loader interface: %p - Loader handle: %p", (void *)iface, (void *)handle);

			if (handle != NULL)
			{
				loader_handle_impl handle_impl = loader_impl_load_handle(impl, iface, handle, subpath);

				if (handle_impl != NULL)
				{
					handle_impl->populated = 1;

					if (set_insert(impl->handle_impl_path_map, handle_impl->path, handle_impl) == 0)
					{
						if (set_insert(impl->handle_impl_map, handle_impl->module, handle_impl) == 0)
						{
							if (iface->discover(impl, handle_impl->module, handle_impl->ctx) == 0)
							{
								if (loader_impl_handle_register(manager, impl, subpath, handle_impl, handle_ptr) == 0)
								{
									vector_push_back_var(impl->handle_impl_init_order, handle_impl);

									return 0;
								}
							}

							set_remove(impl->handle_impl_map, handle_impl->module);
						}

						set_remove(impl->handle_impl_path_map, handle_impl->path);
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
		return (void *)set_get(impl->handle_impl_path_map, (set_key)name);
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

void *loader_impl_handle_container_of(loader_impl impl, void *handle)
{
	if (handle != NULL)
	{
		return (void *)((loader_handle_impl)set_get(impl->handle_impl_map, (set_key)handle));
	}

	return NULL;
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

	value v = value_create_array(NULL, set_size(impl->handle_impl_path_map));

	if (v == NULL)
	{
		return NULL;
	}

	metadata_iterator.iterator = 0;
	metadata_iterator.values = value_to_array(v);

	set_iterate(impl->handle_impl_path_map, &loader_impl_metadata_cb_iterate, (set_cb_iterate_args)&metadata_iterator);

	return v;
}

int loader_impl_clear(void *handle)
{
	if (handle != NULL)
	{
		loader_handle_impl handle_impl = handle;

		loader_impl impl = handle_impl->impl;

		size_t iterator;

		/* Remove the handle from the path indexing set */
		int result = !(set_remove(impl->handle_impl_path_map, (set_key)handle_impl->path) == handle_impl);

		/* Remove the handle from the pointer indexing set */
		result |= !(set_remove(impl->handle_impl_map, (set_key)handle_impl->module) == handle_impl);

		/* Search for the handle in the initialization order list and remove it */
		for (iterator = 0; iterator < vector_size(impl->handle_impl_init_order); ++iterator)
		{
			loader_handle_impl iterator_handle_impl = vector_at_type(impl->handle_impl_init_order, iterator, loader_handle_impl);

			if (handle_impl == iterator_handle_impl)
			{
				vector_erase(impl->handle_impl_init_order, iterator);
				break;
			}
		}

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
		/* Destroy all handles in inverse creation order */
		size_t iterator = vector_size(impl->handle_impl_init_order);

		if (iterator > 0)
		{
			loader_handle_impl iterator_handle_impl;

			do
			{
				--iterator;

				iterator_handle_impl = vector_at_type(impl->handle_impl_init_order, iterator, loader_handle_impl);

				loader_impl_destroy_handle(iterator_handle_impl);

			} while (iterator > 0);
		}

		vector_destroy(impl->handle_impl_init_order);

		/* Destroy the path to handle implementation indexing */
		set_destroy(impl->handle_impl_path_map);

		/* Destroy the handle to handle implementation indexing */
		set_destroy(impl->handle_impl_map);

		/* Destroy all the types */
		set_iterate(impl->type_info_map, &loader_impl_destroy_type_map_cb_iterate, NULL);

		set_destroy(impl->type_info_map);
	}
}

void loader_impl_destroy_deallocate(loader_impl impl)
{
	set_iterate(impl->exec_path_map, &loader_impl_destroy_exec_path_map_cb_iterate, NULL);

	set_destroy(impl->exec_path_map);

	context_destroy(impl->ctx);

	free(impl);
}

void loader_impl_destroy_dtor(plugin p)
{
	loader_impl_destroy(p, plugin_impl_type(p, loader_impl));
}

void loader_impl_destroy(plugin p, loader_impl impl)
{
	if (impl != NULL)
	{
		if (p != NULL)
		{
			if (impl->init == 0)
			{
				loader_impl_interface iface = loader_iface(p);

				log_write("metacall", LOG_LEVEL_DEBUG, "Destroy loader implementation %s", plugin_name(p));

				if (iface != NULL && iface->destroy(impl) != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Invalid loader implementation (%s) interface destruction <%p>", plugin_name(p), iface->destroy);
				}

				impl->init = 1;
			}
		}
		else
		{
			loader_impl_destroy_objects(impl);
		}

		loader_impl_destroy_deallocate(impl);
	}
}
