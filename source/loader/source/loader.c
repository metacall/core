/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

/* -- Headers -- */

#include <metacall/metacall_version.h>

#include <loader/loader.h>
#include <loader/loader_env.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_scope.h>

#include <adt/adt_set.h>
#include <adt/adt_vector.h>

#include <serial/serial.h>

#include <detour/detour.h>

#include <log/log.h>

#include <threading/threading_thread_id.h>

#include <stdlib.h>
#include <string.h>

/* -- Forward Declarations -- */

struct loader_initialization_order_type;

struct loader_get_iterator_args_type;

struct loader_host_invoke_type;

struct loader_metadata_cb_iterator_type;

/* -- Type Definitions -- */

typedef struct loader_initialization_order_type *loader_initialization_order;

typedef struct loader_get_iterator_args_type *loader_get_iterator_args;

typedef struct loader_host_invoke_type *loader_host_invoke;

typedef struct loader_metadata_cb_iterator_type *loader_metadata_cb_iterator;

/* -- Member Data -- */

struct loader_initialization_order_type
{
	uint64_t id;
	loader_impl impl;
	int being_deleted;
};

struct loader_type
{
	loader_impl proxy;			 /* Points to the internal proxy loader */
	set impl_map;				 /* Maps the loader implementations by tag */
	vector initialization_order; /* Stores the loader implementations by order of initialization (used for destruction) */
	uint64_t init_thread_id;	 /* Stores the thread id of the thread that initialized metacall */
};

struct loader_metadata_cb_iterator_type
{
	size_t iterator;
	value *values;
};

struct loader_get_iterator_args_type
{
	const char *name;
	value obj; // scope_object
};

struct loader_host_invoke_type
{
	loader_register_invoke invoke;
};

/* -- Private Methods -- */

static void loader_initialize_proxy(void);

static function_interface loader_register_interface_proxy(void);

static value loader_register_invoke_proxy(function func, function_impl func_impl, function_args args, size_t size);

static function_return loader_register_await_proxy(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context);

static void loader_register_destroy_proxy(function func, function_impl func_impl);

static loader_impl loader_create_impl(const loader_naming_tag tag);

static int loader_get_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

static value loader_metadata_impl(loader_impl impl);

static int loader_metadata_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

/* -- Member Data -- */

static struct loader_type loader_instance_default = {
	NULL, NULL, NULL, THREAD_ID_INVALID
};

static loader loader_instance_ptr = &loader_instance_default;

/* -- Methods -- */

loader loader_singleton()
{
	return loader_instance_ptr;
}

void loader_initialization_register(loader_impl impl)
{
	loader l = loader_singleton();

	if (l->initialization_order != NULL)
	{
		struct loader_initialization_order_type initialization_order;

		initialization_order.id = thread_id_get_current();
		initialization_order.impl = impl;
		initialization_order.being_deleted = 1;

		vector_push_back(l->initialization_order, &initialization_order);
	}
}

void loader_initialize_proxy()
{
	loader l = loader_singleton();

	if (set_get(l->impl_map, (set_key)LOADER_HOST_PROXY_NAME) == NULL)
	{
		l->proxy = loader_impl_create_proxy();

		if (l->proxy != NULL)
		{
			if (set_insert(l->impl_map, (set_key)loader_impl_tag(l->proxy), l->proxy) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Loader invalid proxy insertion <%p>", (void *)l->proxy);

				loader_impl_destroy(l->proxy);
			}

			/* Insert into destruction list */
			loader_initialization_register(l->proxy);

			log_write("metacall", LOG_LEVEL_DEBUG, "Loader proxy initialized");
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Loader invalid proxy initialization");
		}
	}
}

void loader_initialize()
{
	loader l = loader_singleton();

	/* Initialize environment variables */
	loader_env_initialize();

	/* Initialize current thread id */
	if (l->init_thread_id == THREAD_ID_INVALID)
	{
		l->init_thread_id = thread_id_get_current();
	}

	/* Initialize implementation map */
	if (l->impl_map == NULL)
	{
		l->impl_map = set_create(&hash_callback_str, &comparable_callback_str);
	}

	/* Initialize implementation vector */
	if (l->initialization_order == NULL)
	{
		l->initialization_order = vector_create(sizeof(struct loader_initialization_order_type));
	}

	/* Initialize host proxy */
	loader_initialize_proxy();
}

int loader_is_initialized(const loader_naming_tag tag)
{
	loader l = loader_singleton();

	loader_impl impl = (loader_impl)set_get(l->impl_map, (const set_key)tag);

	if (impl == NULL)
	{
		return 1;
	}

	return loader_impl_is_initialized(impl);
}

function_return loader_register_invoke_proxy(function func, function_impl func_impl, function_args args, size_t size)
{
	loader_host_invoke host_invoke = (loader_host_invoke)func_impl;

	void *data = function_closure(func);

	return host_invoke->invoke(size, args, data);
}

function_return loader_register_await_proxy(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)size;
	(void)resolve_callback;
	(void)reject_callback;
	(void)context;

	return NULL;
}

void loader_register_destroy_proxy(function func, function_impl func_impl)
{
	(void)func;

	if (func_impl != NULL)
	{
		free(func_impl);
	}
}

function_interface loader_register_interface_proxy(void)
{
	static struct function_interface_type interface = {
		NULL,
		&loader_register_invoke_proxy,
		&loader_register_await_proxy,
		&loader_register_destroy_proxy
	};

	return &interface;
}

int loader_register(const char *name, loader_register_invoke invoke, function *func, type_id return_type, size_t arg_size, type_id args_type_id[])
{
	static const char register_holder_str[] = "__metacall_register__";

	function f = NULL;

	loader_impl loader = loader_get_impl(LOADER_HOST_PROXY_NAME);

	context ctx = loader_impl_context(loader);

	scope sp = context_scope(ctx);

	function_impl_interface_singleton singleton = &loader_register_interface_proxy;

	loader_host_invoke host_invoke = malloc(sizeof(struct loader_host_invoke_type));

	signature s;

	host_invoke->invoke = invoke;

	f = function_create(name, arg_size, host_invoke, singleton);

	if (f == NULL)
	{
		return 1;
	}

	s = function_signature(f);

	if (arg_size > 0)
	{
		size_t iterator;

		for (iterator = 0; iterator < arg_size; ++iterator)
		{
			signature_set(s, iterator, register_holder_str, type_create(args_type_id[iterator], register_holder_str, NULL, NULL));
		}
	}

	signature_set_return(s, type_create(return_type, register_holder_str, NULL, NULL));

	if (name != NULL)
	{
		scope_define(sp, name, value_create_function(f));
	}

	if (func != NULL)
	{
		*func = f;
	}

	return 0;
}

loader_impl loader_create_impl(const loader_naming_tag tag)
{
	loader l = loader_singleton();

	loader_impl impl = loader_impl_create(loader_env_library_path(), tag);

	if (impl != NULL)
	{
		if (set_insert(l->impl_map, (set_key)loader_impl_tag(impl), impl) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Loader implementation insertion error (%s)", tag);

			loader_impl_destroy(impl);

			return NULL;
		}

		return impl;
	}

	return NULL;
}

loader_impl loader_get_impl(const loader_naming_tag tag)
{
	loader l = loader_singleton();

	loader_impl impl = (loader_impl)set_get(l->impl_map, (const set_key)tag);

	if (impl == NULL)
	{
		impl = loader_create_impl(tag);

		log_write("metacall", LOG_LEVEL_DEBUG, "Created loader (%s) implementation <%p>", tag, (void *)impl);
	}

	return impl;
}

int loader_load_path(const loader_naming_path path)
{
	loader l = loader_singleton();

	loader_initialize();

	if (l->impl_map != NULL)
	{
		(void)path;

		/* ... */
	}

	return 1;
}

int loader_execution_path(const loader_naming_tag tag, const loader_naming_path path)
{
	loader l = loader_singleton();

	loader_initialize();

	if (l->impl_map != NULL)
	{
		/* If loader is initialized, load the execution path */
		loader_impl impl = loader_get_impl(tag);

		log_write("metacall", LOG_LEVEL_DEBUG, "Loader (%s) implementation <%p>", tag, (void *)impl);

		if (impl == NULL)
		{
			return 1;
		}

		return loader_impl_execution_path(impl, path);
	}

	return 1;
}

int loader_load_from_file(const loader_naming_tag tag, const loader_naming_path paths[], size_t size, void **handle)
{
	loader l = loader_singleton();

	loader_initialize();

	if (l->impl_map != NULL)
	{
		if (tag != NULL)
		{
			loader_impl impl = loader_get_impl(tag);

			log_write("metacall", LOG_LEVEL_DEBUG, "Loader (%s) implementation <%p>", tag, (void *)impl);

			if (impl != NULL)
			{
				return loader_impl_load_from_file(impl, paths, size, handle);
			}
		}
	}

	return 1;
}

int loader_load_from_memory(const loader_naming_tag tag, const char *buffer, size_t size, void **handle)
{
	loader l = loader_singleton();

	loader_initialize();

	if (l->impl_map != NULL)
	{
		loader_impl impl = loader_get_impl(tag);

		log_write("metacall", LOG_LEVEL_DEBUG, "Loader (%s) implementation <%p>", tag, (void *)impl);

		if (impl == NULL)
		{
			return 1;
		}

		return loader_impl_load_from_memory(impl, buffer, size, handle);
	}

	return 1;
}

int loader_load_from_package(const loader_naming_tag extension, const loader_naming_path path, void **handle)
{
	loader l = loader_singleton();

	loader_initialize();

	if (l->impl_map != NULL)
	{
		loader_impl impl = loader_get_impl(extension);

		log_write("metacall", LOG_LEVEL_DEBUG, "Loader (%s) implementation <%p>", extension, (void *)impl);

		if (impl == NULL)
		{
			return 1;
		}

		return loader_impl_load_from_package(impl, path, handle);
	}

	return 1;
}

int loader_load_from_configuration(const loader_naming_path path, void **handle, void *allocator)
{
	loader_naming_name config_name;
	configuration config;
	value tag, scripts, context_path;
	value *scripts_array;
	loader_naming_path *paths;
	loader_naming_path context_path_str;
	size_t context_path_size = 0;
	size_t iterator, size;

	if (loader_path_get_name(path, config_name) == 0)
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

	tag = configuration_value(config, "language_id");

	if (tag == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader load from configuration invalid tag (%s)", path);

		configuration_clear(config);

		return 1;
	}

	scripts = configuration_value(config, "scripts");

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

	paths = malloc(sizeof(loader_naming_path) * size);

	if (paths == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader load from configuration invalid paths allocation");

		configuration_clear(config);

		return 1;
	}

	context_path = configuration_value(config, "path");

	if (context_path != NULL)
	{
		const char *str = value_to_string(context_path);

		size_t str_size = value_type_size(context_path);

		loader_naming_path path_base, join_path;

		size_t path_base_size = loader_path_get_path(path, strlen(path) + 1, path_base);

		size_t join_path_size = loader_path_join(path_base, path_base_size, str, str_size, join_path);

		context_path_size = loader_path_canonical(join_path, join_path_size, context_path_str);
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
				(void)loader_path_canonical(str, str_size, paths[iterator]);
			}
			else
			{
				loader_naming_path join_path;

				size_t join_path_size = loader_path_join(context_path_str, context_path_size, str, str_size, join_path);

				(void)loader_path_canonical(join_path, join_path_size, paths[iterator]);
			}
		}
	}

	if (loader_load_from_file((const char *)value_to_string(tag), (const loader_naming_path *)paths, size, handle) != 0)
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

int loader_get_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	(void)s;
	(void)key;

	if (val != NULL && args != NULL)
	{
		loader_impl impl = val;

		loader_get_iterator_args get_args = args;

		context ctx = loader_impl_context(impl);

		scope sp = context_scope(ctx);

		get_args->obj = scope_get(sp, get_args->name);

		if (get_args->obj != NULL)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Loader get callback: impl %p, name %s", (void *)get_args->obj, get_args->name);

			return 1;
		}
	}

	return 0;
}

loader_data loader_get(const char *name)
{
	loader l = loader_singleton();

	if (l->impl_map != NULL)
	{
		struct loader_get_iterator_args_type get_args;

		get_args.name = name;
		get_args.obj = NULL;

		set_iterate(l->impl_map, &loader_get_cb_iterate, (set_cb_iterate_args)&get_args);

		if (get_args.obj != NULL)
		{
			return (loader_data)get_args.obj;
		}
	}

	return NULL;
}

void *loader_get_handle(const loader_naming_tag tag, const char *name)
{
	return loader_impl_get_handle(loader_get_impl(tag), name);
}

void loader_set_options(const loader_naming_tag tag, void *options)
{
	loader_impl_set_options(loader_get_impl(tag), options);
}

void *loader_get_options(const loader_naming_tag tag)
{
	return loader_impl_get_options(loader_get_impl(tag));
}

const char *loader_handle_id(void *handle)
{
	return loader_impl_handle_id(handle);
}

value loader_handle_export(void *handle)
{
	return loader_impl_handle_export(handle);
}

loader_data loader_handle_get(void *handle, const char *name)
{
	if (handle != NULL)
	{
		context ctx = loader_impl_handle_context(handle);

		scope sp = context_scope(ctx);

		return scope_get(sp, name);
	}

	return NULL;
}

value loader_metadata_impl(loader_impl impl)
{
	loader_naming_tag *tag_ptr = loader_impl_tag(impl);

	value *v_ptr, v = value_create_array(NULL, 2);

	if (v == NULL)
	{
		return NULL;
	}

	v_ptr = value_to_array(v);

	v_ptr[0] = value_create_string(*tag_ptr, strlen(*tag_ptr));

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

int loader_metadata_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	loader_metadata_cb_iterator metadata_iterator = (loader_metadata_cb_iterator)args;

	(void)s;
	(void)key;

	metadata_iterator->values[metadata_iterator->iterator] = loader_metadata_impl((loader_impl)val);

	if (metadata_iterator->values[metadata_iterator->iterator] != NULL)
	{
		++metadata_iterator->iterator;
	}

	return 0;
}

value loader_metadata()
{
	loader l = loader_singleton();

	struct loader_metadata_cb_iterator_type metadata_iterator;

	value v;

	if (l->impl_map == NULL)
	{
		return NULL;
	}

	v = value_create_map(NULL, set_size(l->impl_map));

	if (v == NULL)
	{
		return NULL;
	}

	metadata_iterator.iterator = 0;
	metadata_iterator.values = value_to_map(v);

	set_iterate(l->impl_map, &loader_metadata_cb_iterate, (set_cb_iterate_args)&metadata_iterator);

	return v;
}

int loader_clear(void *handle)
{
	return loader_impl_clear(handle);
}

void loader_unload_children(loader_impl impl, int destroy_objects)
{
	loader l = loader_singleton();
	uint64_t current = thread_id_get_current();
	size_t iterator, size = vector_size(l->initialization_order);
	vector stack = vector_create_type(loader_initialization_order);

	/* Get all loaders that have been initialized in the current thread */
	for (iterator = 0; iterator < size; ++iterator)
	{
		loader_initialization_order order = vector_at(l->initialization_order, iterator);

		if (order->being_deleted == 1 && order->impl != NULL && current == order->id)
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

		log_write("metacall", LOG_LEVEL_DEBUG, "Loader unloading (%s)", loader_impl_tag(order->impl));

		/* Call recursively for deletion of children */
		if (order->impl != l->proxy)
		{
			loader_impl_destroy(order->impl);
		}

		/* Clear current order */
		order->being_deleted = 1;
		order->impl = NULL;
		order->id = THREAD_ID_INVALID;

		vector_pop_back(stack);
	}

	vector_destroy(stack);

	/* Clear all objects and types related to the loader once all childs have been destroyed */
	if (impl != NULL && destroy_objects == 0)
	{
		loader_impl_destroy_objects(impl);
	}
}

int loader_unload()
{
	loader l = loader_singleton();

	log_write("metacall", LOG_LEVEL_DEBUG, "Loader begin unload");

	/* Delete loaders in inverse order */
	if (l->initialization_order != NULL)
	{
		uint64_t current = thread_id_get_current();

		if (l->init_thread_id != current)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Destruction of the loaders is being executed "
												   "from different thread of where MetaCall was initialized, "
												   "this is very dangerous and it can generate memory leaks and deadlocks, "
												   "I hope you know what are you doing...");

			/* TODO: How to deal with this? */
		}

		loader_unload_children(NULL, 1);

		/* The proxy is the first loader, it must be destroyed at the end */
		if (l->proxy != NULL)
		{
			loader_impl_destroy_objects(l->proxy);
			loader_impl_destroy(l->proxy);
			l->proxy = NULL;
		}
	}

	/* Clear the implementation tag map */
	if (l->impl_map != NULL)
	{
		if (set_clear(l->impl_map) != 0)
		{
			loader_destroy();

			return 1;
		}
	}

	loader_destroy();

	return 0;
}

void loader_destroy()
{
	loader l = loader_singleton();

	if (l->initialization_order != NULL)
	{
		vector_destroy(l->initialization_order);

		l->initialization_order = NULL;
	}

	if (l->impl_map != NULL)
	{
		set_destroy(l->impl_map);

		l->impl_map = NULL;
	}

	l->init_thread_id = THREAD_ID_INVALID;

	loader_env_destroy();
}

const char *loader_print_info()
{
	static const char loader_info[] =
		"Loader Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef LOADER_STATIC_DEFINE
		"Compiled as static library type\n"
#else
		"Compiled as shared library type\n"
#endif

		"\n";

	return loader_info;
}
