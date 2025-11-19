/*
 *	Loader Library by Parra Studios
 *	A library for loading executable code at run-time into a process.
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

#include <loader/loader.h>
#include <loader/loader_host.h>
#include <loader/loader_manager_impl.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_scope.h>

#include <adt/adt_vector.h>

#include <serial/serial.h>

#include <log/log.h>

#include <threading/threading_thread_id.h>

#include <stdlib.h>
#include <string.h>

/* -- Definitions -- */

#define LOADER_MANAGER_NAME			"loader"
#define LOADER_LIBRARY_PATH			"LOADER_LIBRARY_PATH"
#define LOADER_LIBRARY_DEFAULT_PATH "loaders"

/* -- Private Methods -- */

static void loader_initialization_debug(void);

static void loader_initialization_register_plugin(plugin p);

static plugin loader_get_impl_plugin(const loader_tag tag);

static plugin loader_get_impl_plugin_options(const loader_tag tag, value options);

/* -- Member Data -- */

static plugin_manager_declare(loader_manager);

static int loader_manager_initialized = 1;

/* -- Methods -- */

int loader_initialize(void)
{
	if (loader_manager_initialized == 0)
	{
		return 0;
	}

	loader_manager_impl manager_impl = loader_manager_impl_initialize();

	if (manager_impl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader manager failed to initialize");
		return 1;
	}

	int result = plugin_manager_initialize(
		&loader_manager,
		LOADER_MANAGER_NAME,
		LOADER_LIBRARY_PATH,
#if defined(LOADER_LIBRARY_INSTALL_PATH)
		LOADER_LIBRARY_INSTALL_PATH,
#else
		LOADER_LIBRARY_DEFAULT_PATH,
#endif /* LOADER_LIBRARY_INSTALL_PATH */
		loader_manager_impl_iface(),
		manager_impl);

	if (result != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader plugin manager failed to initialize");
		loader_manager_impl_destroy(manager_impl);
		return 1;
	}

	/* Register host loader */
	if (plugin_manager_register(&loader_manager, manager_impl->host) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader host failed to initialize");
		plugin_destroy(manager_impl->host);
		plugin_manager_destroy(&loader_manager);
		return 1;
	}

	/* Insert into destruction list */
	loader_initialization_register_plugin(manager_impl->host);

	/* Initialize detours */
	manager_impl->d = NULL;

	/* TODO: Disable logs here until log is completely thread safe and async signal safe */
	/* log_write("metacall", LOG_LEVEL_DEBUG, "Loader host initialized"); */

	loader_manager_initialized = 0;

	return 0;
}

void loader_initialization_debug(void)
{
	/* TODO: Disable logs here until log is completely thread safe and async signal safe */
	/*
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	loader_manager_impl manager_impl = plugin_manager_impl_type(&loader_manager, loader_manager_impl);

	if (manager_impl->initialization_order != NULL)
	{
		size_t iterator, size = vector_size(manager_impl->initialization_order);

		log_write("metacall", LOG_LEVEL_DEBUG, "Loader initialization order:\n-----------------------------------");

		for (iterator = 0; iterator < size; ++iterator)
		{
			loader_initialization_order order = vector_at(manager_impl->initialization_order, iterator);

			printf("%" PRIuS ") %s #%" PRIuS "\n", iterator, plugin_name(order->p), order->id);
		}

		fflush(stdout);
	}
#endif
	*/
}

void loader_initialization_register(loader_impl impl)
{
	plugin p = loader_impl_plugin(impl);

	if (p != NULL)
	{
		loader_initialization_register_plugin(p);
		loader_initialization_debug();
	}
}

void loader_initialization_register_plugin(plugin p)
{
	loader_manager_impl manager_impl = plugin_manager_impl_type(&loader_manager, loader_manager_impl);

	if (manager_impl->initialization_order != NULL)
	{
		struct loader_initialization_order_type initialization_order;

		initialization_order.id = thread_id_get_current();
		initialization_order.p = p;
		initialization_order.being_deleted = 1;

		/* TODO: Disable logs here until log is completely thread safe and async signal safe */
		/*
		log_write("metacall", LOG_LEVEL_DEBUG, "Loader %s registered at position (%" PRIuS ") in thread #%" PRIuS,
			plugin_name(p), vector_size(manager_impl->initialization_order), initialization_order.id);
		*/

		vector_push_back(manager_impl->initialization_order, &initialization_order);
	}
}

int loader_is_initialized(const loader_tag tag)
{
	plugin p = plugin_manager_get(&loader_manager, tag);

	if (p == NULL)
	{
		return 1;
	}

	return loader_impl_is_initialized(plugin_impl_type(p, loader_impl));
}

int loader_register(const char *name, loader_register_invoke invoke, function *func, type_id return_type, size_t arg_size, type_id args_type_id[], void *data)
{
	loader_manager_impl manager_impl = plugin_manager_impl_type(&loader_manager, loader_manager_impl);

	return loader_host_register(plugin_impl_type(manager_impl->host, loader_impl), NULL, name, invoke, func, return_type, arg_size, args_type_id, data);
}

int loader_register_handle(void *impl, void *handle, const char *name, loader_register_invoke invoke, type_id return_type, size_t arg_size, type_id args_type_id[], void *data)
{
	return loader_host_register((loader_impl)impl, loader_impl_handle_context(handle), name, invoke, NULL, return_type, arg_size, args_type_id, data);
}

void loader_detour(detour d)
{
	loader_manager_impl manager_impl = plugin_manager_impl_type(&loader_manager, loader_manager_impl);

	manager_impl->d = d;
}

detour_handle loader_hook(const loader_tag tag, const char *library, int (*load_cb)(detour, detour_handle))
{
	return loader_impl_detour(loader_get_impl(tag), library, load_cb);
}

detour_handle loader_hook_impl(void *impl, const char *library, int (*load_cb)(detour, detour_handle))
{
	return loader_impl_detour((loader_impl)impl, library, load_cb);
}

plugin loader_get_impl_plugin(const loader_tag tag)
{
	return loader_get_impl_plugin_options(tag, NULL);
}

plugin loader_get_impl_plugin_options(const loader_tag tag, value options)
{
	plugin p = plugin_manager_get(&loader_manager, tag);

	loader_impl impl;

	if (p != NULL)
	{
		return p;
	}

	impl = loader_impl_create(tag);

	if (impl == NULL)
	{
		/* Destroy options on error */
		value_type_destroy(options);
		goto loader_create_error;
	}

	/* Define the options */
	loader_impl_set_options(impl, options);

	/* Dynamic link loader dependencies if it is not host */
	if (loader_impl_dependencies(impl, plugin_manager_impl_type(&loader_manager, loader_manager_impl)->d, tag) != 0)
	{
		goto plugin_manager_create_error;
	}

	/* Dynamic link the loader */
	p = plugin_manager_create(&loader_manager, tag, impl, &loader_impl_destroy_dtor);

	if (p == NULL)
	{
		goto plugin_manager_create_error;
	}

	/* If it is host, link the loader symbols to the host (either the executable or a library) */
	if (loader_impl_link(p, impl) != 0)
	{
		goto plugin_manager_create_error;
	}

	/* Store in the loader implementation the reference to the plugin which belongs to */
	loader_impl_attach(impl, p);

	/* Check if it is host, initialize it and set it as host */
	if (options != NULL && loader_impl_get_option_host(impl) == 1)
	{
		loader_manager_impl manager_impl;

		if (loader_impl_initialize(&loader_manager, p, plugin_impl_type(p, loader_impl)) != 0)
		{
			goto plugin_manager_create_error;
		}

		manager_impl = plugin_manager_impl_type(&loader_manager, loader_manager_impl);

		manager_impl->host = p;
	}

	/* TODO: Disable logs here until log is completely thread safe and async signal safe */
	/* log_write("metacall", LOG_LEVEL_DEBUG, "Created loader (%s) implementation <%p>", tag, (void *)impl); */

	return p;

plugin_manager_create_error:
	loader_impl_destroy(p, impl);
loader_create_error:
	log_write("metacall", LOG_LEVEL_ERROR, "Failed to create loader: %s", tag);
	return NULL;
}

loader_impl loader_get_impl(const loader_tag tag)
{
	plugin p = loader_get_impl_plugin(tag);

	return plugin_impl_type(p, loader_impl);
}

const char *loader_library_path(void)
{
	return plugin_manager_library_path(&loader_manager);
}

int loader_execution_path(const loader_tag tag, const loader_path path)
{
	if (loader_initialize() == 1)
	{
		return 1;
	}

	plugin p = loader_get_impl_plugin(tag);

	if (p == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Tried to define an execution path from non existent loader (%s): %s", tag, path);
		return 1;
	}

	/* TODO: Disable logs here until log is completely thread safe and async signal safe */
	/* log_write("metacall", LOG_LEVEL_DEBUG, "Define execution path (%s): %s", tag, path); */

	return loader_impl_execution_path(p, plugin_impl_type(p, loader_impl), path);
}

int loader_load_from_file(const loader_tag tag, const loader_path paths[], size_t size, void **handle)
{
	if (loader_initialize() == 1)
	{
		return 1;
	}

	plugin p = loader_get_impl_plugin(tag);

	if (p == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Tried to load %" PRIuS " file(s) from non existent loader (%s): %s", size, tag, paths[0]);
		return 1;
	}

	/* TODO: Disable logs here until log is completely thread safe and async signal safe */
	/* log_write("metacall", LOG_LEVEL_DEBUG, "Loading %" PRIuS " file(s) (%s) from path(s): %s ...", size, tag, paths[0]); */

	return loader_impl_load_from_file(&loader_manager, p, plugin_impl_type(p, loader_impl), paths, size, handle);
}

int loader_load_from_memory(const loader_tag tag, const char *buffer, size_t size, void **handle)
{
	if (loader_initialize() == 1)
	{
		return 1;
	}

	plugin p = loader_get_impl_plugin(tag);

	if (p == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Tried to load a buffer from non existent loader (%s): %s", tag, buffer);
		return 1;
	}

	/* TODO: Disable logs here until log is completely thread safe and async signal safe */
	/* log_write("metacall", LOG_LEVEL_DEBUG, "Loading buffer from memory (%s):\n%s", tag, buffer); */

	return loader_impl_load_from_memory(&loader_manager, p, plugin_impl_type(p, loader_impl), buffer, size, handle);
}

int loader_load_from_package(const loader_tag tag, const loader_path path, void **handle)
{
	if (loader_initialize() == 1)
	{
		return 1;
	}

	plugin p = loader_get_impl_plugin(tag);

	if (p == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Tried to load a package from non existent loader (%s): %s", tag, path);
		return 1;
	}

	/* TODO: Disable logs here until log is completely thread safe and async signal safe */
	/* log_write("metacall", LOG_LEVEL_DEBUG, "Loading package (%s): %s", tag, path); */

	return loader_impl_load_from_package(&loader_manager, p, plugin_impl_type(p, loader_impl), path, handle);
}

int loader_load_from_configuration(const loader_path path, void **handle, void *allocator)
{
	loader_name config_name;
	configuration config;
	value tag, scripts, context_path;
	value *scripts_array;
	loader_path *paths;
	loader_path context_path_str;
	size_t context_path_size = 0;
	size_t iterator, size;

	if (loader_initialize() == 1)
	{
		return 1;
	}

	if (portability_path_get_name(path, strnlen(path, LOADER_PATH_SIZE) + 1, config_name, LOADER_NAME_SIZE) == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader load from configuration invalid config name (%s)", path);

		return 1;
	}

	config = configuration_create(config_name, path, NULL, allocator);

	if (config == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader load from configuration invalid initialization (%s)", path);

		return 1;
	}

	tag = configuration_value_type(config, "language_id", TYPE_STRING);

	if (tag == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader load from configuration invalid tag (%s)", path);

		configuration_clear(config);

		return 1;
	}

	scripts = configuration_value_type(config, "scripts", TYPE_ARRAY);

	if (scripts == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader load from configuration invalid scripts (%s)", path);

		configuration_clear(config);

		return 1;
	}

	size = value_type_count(scripts);

	if (size == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader load from configuration cannot load zero scripts");

		configuration_clear(config);

		return 1;
	}

	paths = malloc(sizeof(loader_path) * size);

	if (paths == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader load from configuration invalid paths allocation");

		configuration_clear(config);

		return 1;
	}

	context_path = configuration_value_type(config, "path", TYPE_STRING);

	if (context_path != NULL)
	{
		const char *str = value_to_string(context_path);

		size_t str_size = value_type_size(context_path);

		if (portability_path_is_absolute(str, str_size) == 0)
		{
			context_path_size = portability_path_canonical(str, str_size, context_path_str, LOADER_PATH_SIZE);
		}
		else
		{
			loader_path path_base, join_path;

			size_t path_base_size = portability_path_get_directory(path, strnlen(path, LOADER_PATH_SIZE) + 1, path_base, LOADER_PATH_SIZE);

			size_t join_path_size = portability_path_join(path_base, path_base_size, str, str_size, join_path, LOADER_PATH_SIZE);

			context_path_size = portability_path_canonical(join_path, join_path_size, context_path_str, LOADER_PATH_SIZE);
		}
	}

	scripts_array = value_to_array(scripts);

	for (iterator = 0; iterator < size; ++iterator)
	{
		if (scripts_array[iterator] != NULL)
		{
			const char *str = value_to_string(scripts_array[iterator]);

			size_t str_size = value_type_size(scripts_array[iterator]);

			if (context_path == NULL)
			{
				(void)portability_path_canonical(str, str_size, paths[iterator], LOADER_PATH_SIZE);
			}
			else
			{
				loader_path join_path;

				size_t join_path_size = portability_path_join(context_path_str, context_path_size, str, str_size, join_path, LOADER_PATH_SIZE);

				(void)portability_path_canonical(join_path, join_path_size, paths[iterator], LOADER_PATH_SIZE);
			}
		}
	}

	if (loader_load_from_file((const char *)value_to_string(tag), (const loader_path *)paths, size, handle) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader load from configuration invalid load from file");

		configuration_clear(config);

		free(paths);

		return 1;
	}

	configuration_clear(config);

	free(paths);

	return 0;
}

value loader_get(const char *name)
{
	struct set_iterator_type it;

	for (set_iterator_begin(&it, loader_manager.plugins); set_iterator_end(&it) != 0; set_iterator_next(&it))
	{
		plugin p = set_iterator_value(&it);

		loader_impl impl = plugin_impl_type(p, loader_impl);

		value scope_object = loader_impl_get_value(impl, name);

		if (scope_object != NULL)
		{
			return scope_object;
		}
	}

	return NULL;
}

void *loader_get_handle(const loader_tag tag, const char *name)
{
	plugin p = loader_get_impl_plugin(tag);

	return loader_impl_get_handle(plugin_impl_type(p, loader_impl), name);
}

int loader_set_options(const loader_tag tag, value options)
{
	plugin p = loader_get_impl_plugin_options(tag, options);

	return (p == NULL);
}

value loader_get_options(const loader_tag tag)
{
	plugin p = loader_get_impl_plugin(tag);

	return loader_impl_get_options(plugin_impl_type(p, loader_impl));
}

value loader_get_option(const loader_tag tag, const char *field)
{
	plugin p = loader_get_impl_plugin(tag);

	return loader_impl_get_option(plugin_impl_type(p, loader_impl), field);
}

int loader_handle_initialize(loader_impl impl, const loader_path name, void **handle_ptr)
{
	if (loader_initialize() == 1)
	{
		return 1;
	}

	plugin p = loader_impl_plugin(impl);

	return loader_impl_handle_initialize(&loader_manager, p, impl, name, handle_ptr);
}

int loader_handle_populate(void *handle_dest, void *handle_src)
{
	context ctx_dest = loader_impl_handle_context(handle_dest);
	context ctx_src = loader_impl_handle_context(handle_src);
	char *duplicated_key;

	if (context_contains(ctx_src, ctx_dest, &duplicated_key) == 0 && duplicated_key != NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Duplicated symbol found named '%s' already defined in the handle scope", duplicated_key);
		return 1;
	}
	else if (context_append(ctx_dest, ctx_src) == 0)
	{
		vector_push_back_var(loader_impl_handle_populated(handle_src), handle_dest);

		return 0;
	}

	return 1;
}

const char *loader_handle_id(void *handle)
{
	return loader_impl_handle_id(handle);
}

value loader_handle_export(void *handle)
{
	return loader_impl_handle_export(handle);
}

value loader_handle_get(void *handle, const char *name)
{
	if (handle != NULL)
	{
		context ctx = loader_impl_handle_context(handle);

		scope sp = context_scope(ctx);

		return scope_get(sp, name);
	}

	return NULL;
}

value loader_metadata_impl(plugin p, loader_impl impl)
{
	const char *tag = plugin_name(p);

	value *v_ptr, v = value_create_array(NULL, 2);

	if (v == NULL)
	{
		return NULL;
	}

	v_ptr = value_to_array(v);

	v_ptr[0] = value_create_string(tag, strnlen(tag, LOADER_TAG_SIZE));

	if (v_ptr[0] == NULL)
	{
		value_type_destroy(v);

		return NULL;
	}

	v_ptr[1] = loader_impl_metadata(impl);

	if (v_ptr[1] == NULL)
	{
		value_type_destroy(v);

		return NULL;
	}

	return v;
}

value loader_metadata(void)
{
	value *values, v = value_create_map(NULL, plugin_manager_size(&loader_manager));
	struct set_iterator_type it;
	size_t values_it;

	if (v == NULL)
	{
		return NULL;
	}

	values = value_to_map(v);

	for (set_iterator_begin(&it, loader_manager.plugins), values_it = 0; set_iterator_end(&it) != 0; set_iterator_next(&it))
	{
		plugin p = set_iterator_value(&it);

		loader_impl impl = plugin_impl_type(p, loader_impl);

		values[values_it] = loader_metadata_impl(p, impl);

		if (values[values_it] != NULL)
		{
			++values_it;
		}
	}

	return v;
}

int loader_clear(void *handle)
{
	return loader_impl_clear(handle);
}

int loader_is_destroyed(loader_impl impl)
{
	loader_manager_impl manager_impl = plugin_manager_impl_type(&loader_manager, loader_manager_impl);

	return loader_manager_impl_is_destroyed(manager_impl, impl);
}

void loader_set_destroyed(loader_impl impl)
{
	loader_manager_impl manager_impl = plugin_manager_impl_type(&loader_manager, loader_manager_impl);

	loader_manager_impl_set_destroyed(manager_impl, impl);
}

void loader_unload_children(loader_impl impl)
{
	loader_manager_impl manager_impl = plugin_manager_impl_type(&loader_manager, loader_manager_impl);
	uint64_t current = thread_id_get_current();
	size_t iterator, size = vector_size(manager_impl->initialization_order);
	vector stack = vector_create_type(loader_initialization_order);

	/* Get all loaders that have been initialized in the current thread */
	for (iterator = 0; iterator < size; ++iterator)
	{
		loader_initialization_order order = vector_at(manager_impl->initialization_order, iterator);

		if (order->being_deleted == 1 && order->p != NULL && current == order->id)
		{
			/* Mark for deletion */
			vector_push_back(stack, &order);

			/* Start to delete the current loader */
			order->being_deleted = 0;
		}
	}

	/* Free all loaders of the current thread and with BFS, look for children */
	while (vector_size(stack) != 0)
	{
		loader_initialization_order order = vector_back_type(stack, loader_initialization_order);

		/* TODO: Disable logs here until log is completely thread safe and async signal safe */
		/* log_write("metacall", LOG_LEVEL_DEBUG, "Loader unloading (%s) from thread #%" PRIuS, plugin_name(order->p), order->id); */

		/* Call recursively for deletion of children */
		if (order->p != manager_impl->host)
		{
			loader_impl destroyed_impl = plugin_impl_type(order->p, loader_impl);

			/* This will execute the desctructor but it will prevent to unload the plugins.
			* This is necessary in order to protect the destruction of the memory, otherwhise
			* it can happen that a node function wrapping a python function get destroyed after
			* the loader has been unloaded, and the function interface will point to an unloaded
			* plugin, generating a segmentation fault. All the plugins will be unloaded on plugin_manager_destroy.
			*/
			plugin_destructor(order->p);

			/* Mark loader as destroyed (prevents access to already freed memory and defines what loaders are destroyed) */
			loader_manager_impl_set_destroyed(manager_impl, destroyed_impl);
		}

		/* Clear current order */
		order->being_deleted = 1;
		order->p = NULL;
		order->id = THREAD_ID_INVALID;

		vector_pop_back(stack);
	}

	vector_destroy(stack);

	/* Clear all objects and types related to the loader once all childs have been destroyed */
	if (impl != NULL)
	{
		loader_impl_destroy_objects(impl);
	}
}

void loader_destroy(void)
{
	loader_manager_impl manager_impl = plugin_manager_impl_type(&loader_manager, loader_manager_impl);

	/* TODO: Disable logs here until log is completely thread safe and async signal safe */
	/* log_write("metacall", LOG_LEVEL_DEBUG, "Begin to destroy all the loaders"); */

	/* Delete loaders in inverse order */
	if (manager_impl->initialization_order != NULL)
	{
		uint64_t current = thread_id_get_current();

		if (manager_impl->init_thread_id != current)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Destruction of the loaders is being executed "
												   "from different thread of where MetaCall was initialized, "
												   "this is very dangerous and it can generate memory leaks and deadlocks, "
												   "I hope you know what are you doing...");

			/* TODO: How to deal with this? */
		}

		loader_initialization_debug();

		loader_unload_children(NULL);

		/* The host is the first loader, it must be destroyed at the end */
		if (manager_impl->host == loader_host_get())
		{
			if (plugin_manager_clear(&loader_manager, manager_impl->host) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Failed to clear host loader");
			}
		}
		else
		{
			plugin_destroyed(manager_impl->host);
		}

		manager_impl->host = NULL;
	}

	plugin_manager_destroy(&loader_manager);

	loader_manager_initialized = 1;
}

const char *loader_print_info(void)
{
	static const char loader_info[] =
		"Loader Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef LOADER_STATIC_DEFINE
		"Compiled as static library type\n"
#else
		"Compiled as shared library type\n"
#endif

		"\n";

	return loader_info;
}
