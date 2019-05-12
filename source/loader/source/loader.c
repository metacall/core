/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

 /* -- Headers -- */

#include <metacall/metacall_version.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

#include <adt/adt_set.h>

#include <environment/environment_variable_path.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

/* -- Definitions -- */

#define LOADER_LIBRARY_PATH			"LOADER_LIBRARY_PATH"
#define LOADER_LIBRARY_DEFAULT_PATH	"loaders"

#define LOADER_SCRIPT_PATH			"LOADER_SCRIPT_PATH"
#define LOADER_SCRIPT_DEFAULT_PATH	"."

/* -- Forward Declarations -- */

struct loader_get_iterator_args_type;

struct loader_host_invoke_type;

struct loader_metadata_cb_iterator_type;

/* -- Type Definitions -- */

typedef struct loader_get_iterator_args_type * loader_get_iterator_args;

typedef struct loader_host_invoke_type * loader_host_invoke;

typedef struct loader_metadata_cb_iterator_type * loader_metadata_cb_iterator;

/* -- Member Data -- */

struct loader_type
{
	set impl_map;
	char * library_path;
	char * script_path;
};

struct loader_metadata_cb_iterator_type
{
	size_t iterator;
	value * values;
};

struct loader_get_iterator_args_type
{
	const char * name;
	scope_object obj;
};

struct loader_host_invoke_type
{
	loader_register_invoke invoke;
};

/* -- Private Methods -- */

static function_interface loader_register_interface_proxy(void);

static value loader_register_invoke_proxy(function func, function_impl func_impl, function_args args);

static void loader_register_destroy_proxy(function func, function_impl func_impl);

static loader_impl loader_create_impl(const loader_naming_tag tag);

static int loader_get_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

static value loader_metadata_impl(loader_impl impl);

static int loader_metadata_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

static int loader_unload_impl_map_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

/* -- Methods -- */

loader loader_singleton()
{
	static struct loader_type loader_instance =
	{
		NULL, NULL, NULL
	};

	return &loader_instance;
}

void loader_initialize()
{
	loader l = loader_singleton();

	if (l->impl_map == NULL)
	{
		l->impl_map = set_create(&hash_callback_str, &comparable_callback_str);
	}

	if (l->library_path == NULL)
	{
		#if defined(LOADER_LIBRARY_INSTALL_PATH)
			static const char loader_library_default_path[] = LOADER_LIBRARY_INSTALL_PATH;
		#else
			static const char loader_library_default_path[] = LOADER_LIBRARY_DEFAULT_PATH;
		#endif /* LOADER_LIBRARY_INSTALL_PATH */

		l->library_path = environment_variable_path_create(LOADER_LIBRARY_PATH, loader_library_default_path);

		log_write("metacall", LOG_LEVEL_DEBUG, "Loader library path: %s", l->library_path);
	}

	if (l->script_path == NULL)
	{
		static const char loader_script_default_path[] = LOADER_SCRIPT_DEFAULT_PATH;

		l->script_path = environment_variable_path_create(LOADER_SCRIPT_PATH, loader_script_default_path);

		log_write("metacall", LOG_LEVEL_DEBUG, "Loader script path: %s", l->script_path);
	}

	if (set_get(l->impl_map, (set_key)LOADER_HOST_PROXY_NAME) == NULL)
	{
		loader_host host = (loader_host)malloc(sizeof(struct loader_host_type));

		if (host != NULL)
		{
			loader_impl proxy;

			host->log = log_instance();

			proxy = loader_impl_create_proxy(host);

			if (proxy != NULL)
			{
				if (set_insert(l->impl_map, (set_key)loader_impl_tag(proxy), proxy) != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Loader invalid proxy insertion <%p>", (void *) proxy);

					loader_impl_destroy(proxy);

					free(host);
				}

				log_write("metacall", LOG_LEVEL_DEBUG, "Loader proxy initialized");
			}
			else
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Loader invalid proxy initialization");

				free(host);
			}
		}
	}
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

value loader_register_invoke_proxy(function func, function_impl func_impl, function_args args)
{
	loader_host_invoke host_invoke = (loader_host_invoke)func_impl;

	(void)func;

	return host_invoke->invoke(args);
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
	static struct function_interface_type interface =
	{
		NULL,
		&loader_register_invoke_proxy,
		&loader_register_destroy_proxy
	};

	return &interface;
}

int loader_register(const char * name, loader_register_invoke invoke, type_id return_type, size_t arg_size, type_id args_type_id[])
{
	function f = NULL;

	loader_impl loader = loader_get_impl(LOADER_HOST_PROXY_NAME);

	context ctx = loader_impl_context(loader);

	scope sp = context_scope(ctx);

	function_impl_interface_singleton singleton = &loader_register_interface_proxy;

	loader_host_invoke host_invoke = malloc(sizeof(struct loader_host_invoke_type));

	host_invoke->invoke = invoke;

	f = function_create(name, arg_size, host_invoke, singleton);

	if (f != NULL)
	{
		/* TODO: Change this to the correct type system */
		const char register_holder_str[] = "__metacall_register__";

		signature s = function_signature(f);

		if (arg_size > 0)
		{
			size_t iterator;

			for (iterator = 0; iterator < arg_size; ++iterator)
			{
				signature_set(s, iterator, register_holder_str, type_create(args_type_id[iterator], register_holder_str, NULL, NULL));
			}
		}

		signature_set_return(s, type_create(return_type, register_holder_str, NULL, NULL));

		scope_define(sp, name, f);

		return 0;
	}

	return 1;
}

loader_impl loader_create_impl(const loader_naming_tag tag)
{
	loader l = loader_singleton();

	loader_impl impl;

	loader_host host = (loader_host)malloc(sizeof(struct loader_host_type));

	host->log = log_instance();

	impl = loader_impl_create(l->library_path, tag, host);

	if (impl != NULL)
	{
		if (set_insert(l->impl_map, (set_key)loader_impl_tag(impl), impl) == 0)
		{
			return impl;
		}

		log_write("metacall", LOG_LEVEL_ERROR, "Loader implementation insertion error (%s)", tag);

		loader_impl_destroy(impl);
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

int loader_load_from_file(const loader_naming_tag tag, const loader_naming_path paths[], size_t size, void ** handle)
{
	loader l = loader_singleton();

	loader_initialize();

	if (l->impl_map != NULL && size > 0 && size < LOADER_LOAD_FROM_FILES_SIZE)
	{
		if (tag != NULL)
		{
			loader_impl impl = loader_get_impl(tag);

			log_write("metacall", LOG_LEVEL_DEBUG, "Loader (%s) implementation <%p>", tag, (void *)impl);

			if (impl != NULL)
			{
				if (l->script_path != NULL)
				{
					loader_naming_path * absolute_paths = malloc(sizeof(loader_naming_path) * size);

					size_t iterator;

					int result;

					if (absolute_paths == NULL)
					{
						log_write("metacall", LOG_LEVEL_ERROR, "Loader load from file invalid absolute paths allocation");

						return 1;
					}

					for (iterator = 0; iterator < size; ++iterator)
					{
						if (loader_path_is_absolute(paths[iterator]) != 0)
						{
							(void)loader_path_join(l->script_path, strlen(l->script_path) + 1, paths[iterator], strnlen(paths[iterator], LOADER_NAMING_PATH_SIZE - 1) + 1, absolute_paths[iterator]);
						}
						else
						{
							strncpy(absolute_paths[iterator], paths[iterator], strnlen(paths[iterator], LOADER_NAMING_PATH_SIZE - 1) + 1);
						}
					}

					result = loader_impl_load_from_file(impl, (const loader_naming_path *)absolute_paths, size, handle);

					free(absolute_paths);

					return result;
				}
				else
				{
					return loader_impl_load_from_file(impl, paths, size, handle);
				}
			}
		}
	}

	return 1;
}

int loader_load_from_memory(const loader_naming_tag tag, const char * buffer, size_t size, void ** handle)
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

int loader_load_from_package(const loader_naming_tag extension, const loader_naming_path path, void ** handle)
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

int loader_load_from_configuration(const loader_naming_path path, void ** handle, void * allocator)
{
	loader_naming_name config_name;

	configuration config;

	value tag, scripts, context_path;

	value * scripts_array;

	loader_naming_path * paths;

	loader_naming_path context_path_str;

	size_t context_path_size;

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

	if (size > LOADER_LOAD_FROM_FILES_SIZE)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader load from configuration too many files (%" PRIuS ")", size);

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
		const char * str = value_to_string(context_path);

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
			const char * str = value_to_string(scripts_array[iterator]);

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

loader_data loader_get(const char * name)
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

void * loader_get_handle(const loader_naming_tag tag, const char * name)
{
	return loader_impl_get_handle(loader_get_impl(tag), name);
}

void loader_set_options(const loader_naming_tag tag, void * options)
{
	loader_impl_set_options(loader_get_impl(tag), options);
}

void * loader_get_options(const loader_naming_tag tag)
{
	return loader_impl_get_options(loader_get_impl(tag));
}

const char * loader_handle_id(void * handle)
{
	return loader_impl_handle_id(handle);
}

value loader_metadata_impl(loader_impl impl)
{
	loader_naming_tag * tag_ptr = loader_impl_tag(impl);

	value * v_ptr, v = value_create_array(NULL, 2);

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

int loader_unload_impl_map_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	(void)s;
	(void)key;
	(void)args;

	if (val != NULL)
	{
		loader_impl impl = val;

		log_write("metacall", LOG_LEVEL_DEBUG, "Loader unloading (%s)", loader_impl_tag(impl));

		loader_impl_destroy(impl);

		return 0;
	}

	return 1;
}

int loader_clear(void * handle)
{
	return loader_impl_clear(handle);
}

int loader_unload()
{
	loader l = loader_singleton();

	log_write("metacall", LOG_LEVEL_DEBUG, "Loader begin unload");

	if (l->impl_map != NULL)
	{
		set_iterate(l->impl_map, &loader_unload_impl_map_cb_iterate, NULL);

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

	if (l->impl_map != NULL)
	{
		set_destroy(l->impl_map);

		l->impl_map = NULL;
	}

	if (l->library_path != NULL)
	{
		environment_variable_path_destroy(l->library_path);

		l->library_path = NULL;
	}

	if (l->script_path != NULL)
	{
		environment_variable_path_destroy(l->script_path);

		l->script_path = NULL;
	}
}

const char * loader_print_info()
{
	static const char loader_info[] =
		"Loader Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef LOADER_STATIC_DEFINE
			"Compiled as static library type\n"
		#else
			"Compiled as shared library type\n"
		#endif

		"\n";

	return loader_info;
}
