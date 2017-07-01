/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

 /* -- Headers -- */

#include <metacall/metacall-version.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

#include <adt/adt_hash_map.h>

#include <environment/environment_variable_path.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

/* -- Definitions -- */

#define LOADER_LIBRARY_PATH			"LOADER_LIBRARY_PATH"
#define LOADER_DEFAULT_LIBRARY_PATH	"."

#define LOADER_SCRIPT_PATH			"LOADER_SCRIPT_PATH"
#define LOADER_DEFAULT_SCRIPT_PATH	"scripts"

/* -- Forward Declarations -- */

struct loader_get_iterator_args_type;

struct loader_host_invoke_type;

/* -- Type Definitions -- */

typedef struct loader_get_iterator_args_type * loader_get_iterator_args;

typedef struct loader_host_invoke_type * loader_host_invoke;

/* -- Member Data -- */

struct loader_type
{
	hash_map impl_map;
	char * library_path;
	char * script_path;
};

struct loader_inspect_cb_iterate_type
{
	char * buffer;
	size_t size;
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

static loader_impl loader_create_impl(const loader_naming_tag extension);

static int loader_get_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args);

static int loader_inspect_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args);

static int loader_unload_impl_map_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args);

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
		l->impl_map = hash_map_create(&hash_callback_str, &comparable_callback_str);
	}

	if (l->library_path == NULL)
	{
		static const char loader_library_default_path[] = LOADER_DEFAULT_LIBRARY_PATH;

		l->library_path = environment_variable_path_create(LOADER_LIBRARY_PATH, loader_library_default_path);

		log_write("metacall", LOG_LEVEL_DEBUG, "Loader library path: %s", l->library_path);
	}

	if (l->script_path == NULL)
	{
		static const char loader_script_default_path[] = LOADER_DEFAULT_SCRIPT_PATH;

		l->script_path = environment_variable_path_create(LOADER_SCRIPT_PATH, loader_script_default_path);

		log_write("metacall", LOG_LEVEL_DEBUG, "Loader script path: %s", l->script_path);
	}

	if (hash_map_get(l->impl_map, (hash_map_key)LOADER_HOST_PROXY_NAME) == NULL)
	{
		loader_impl proxy = loader_impl_create_proxy();

		if (proxy == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Loader invalid proxy initialization");
		}

		if (hash_map_insert(l->impl_map, (hash_map_key)LOADER_HOST_PROXY_NAME, proxy) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Loader invalid proxy insertion <%p>", (void *) proxy);

			loader_impl_destroy(proxy);
		}
	}
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

	loader_impl impl = loader_impl_create(l->library_path, (const hash_map_key)tag);

	if (impl != NULL)
	{
		if (hash_map_insert(l->impl_map, (hash_map_key)tag, impl) == 0)
		{
			if (loader_impl_execution_path(impl, ".") == 0)
			{
				if (l->script_path != NULL)
				{
					if (loader_impl_execution_path(impl, l->script_path) == 0)
					{
						return impl;
					}
				}
				else
				{
					return impl;
				}
			}

			hash_map_remove(l->impl_map, (hash_map_key)tag);
		}

		loader_impl_destroy(impl);
	}

	return NULL;
}

loader_impl loader_get_impl(const loader_naming_tag tag)
{
	loader l = loader_singleton();

	loader_impl impl = (loader_impl)hash_map_get(l->impl_map, (const hash_map_key)tag);

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

	#ifdef LOADER_LAZY
		log_write("metacall", LOG_LEVEL_DEBUG, "Loader lazy initialization");

		loader_initialize();
	#endif

	if (l->impl_map != NULL)
	{
		(void)path;

		/* ... */
	}

	return 1;
}

int loader_load_from_file(const loader_naming_tag tag, const loader_naming_path paths[], size_t size)
{
	loader l = loader_singleton();

	#ifdef LOADER_LAZY
		log_write("metacall", LOG_LEVEL_DEBUG, "Loader lazy initialization");

		loader_initialize();
	#endif

	if (l->impl_map != NULL && size > 0)
	{
		if (tag != NULL)
		{
			loader_impl impl = loader_get_impl(tag);

			log_write("metacall", LOG_LEVEL_DEBUG, "Loader (%s) implementation <%p>", tag, (void *)impl);

			if (impl != NULL)
			{
				if (l->script_path != NULL)
				{
					loader_naming_path absolute_path[LOADER_LOAD_FROM_FILES_SIZE];

					size_t iterator;

					for (iterator = 0; iterator < size; ++iterator)
					{
						memcpy(absolute_path[iterator], l->script_path, strlen(l->script_path) + 1);

						strncat(absolute_path[iterator], paths[iterator], LOADER_NAMING_PATH_SIZE - 1);
					}

					return loader_impl_load_from_file(impl, (const loader_naming_path *)absolute_path, size);
				}
				else
				{
					return loader_impl_load_from_file(impl, paths, size);
				}
			}
		}
	}

	return 1;
}

int loader_load_from_memory(const loader_naming_tag tag, const char * buffer, size_t size)
{
	loader l = loader_singleton();

	#ifdef LOADER_LAZY
		log_write("metacall", LOG_LEVEL_DEBUG, "Loader lazy initialization");

		loader_initialize();
	#endif

	if (l->impl_map != NULL)
	{
		loader_impl impl = loader_get_impl(tag);

		log_write("metacall", LOG_LEVEL_DEBUG, "Loader (%s) implementation <%p>", tag, (void *)impl);

		if (impl == NULL)
		{
			return 1;
		}
		return loader_impl_load_from_memory(impl, buffer, size);
	}

	return 1;
}

int loader_load_from_package(const loader_naming_tag extension, const loader_naming_path path)
{
	loader l = loader_singleton();

	#ifdef LOADER_LAZY
		log_write("metacall", LOG_LEVEL_DEBUG, "Loader lazy initialization");

		loader_initialize();
	#endif

	if (l->impl_map != NULL)
	{
		loader_impl impl = loader_get_impl(extension);

		log_write("metacall", LOG_LEVEL_DEBUG, "Loader (%s) implementation <%p>", extension, (void *)impl);

		if (impl == NULL)
		{
			return 1;
		}

		return loader_impl_load_from_package(impl, path);
	}

	return 1;
}

int loader_load_from_configuration(const loader_naming_path path)
{
	loader_naming_name config_name;

	configuration config;

	value tag, scripts;

	value * scripts_array;

	loader_naming_path * paths;

	size_t index, size;

	if (loader_path_get_name(path, config_name) == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader load from configuration invalid config name (%s)", path);

		return 1;
	}

	config = configuration_create(config_name, path, NULL);

	if (config == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader load from configuration invalid initialization (%s)", path);

		return 1;
	}

	tag = configuration_value(config, "tag");

	if (tag == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader load from configuration invalid tag (%s)", path);

		configuration_clear(config);

		return 1;
	}

	scripts = configuration_value(config, "scripts");

	if (tag == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader load from configuration invalid scripts (%s)", path);

		configuration_clear(config);

		return 1;
	}

	size = value_type_size(scripts) / sizeof(value);

	paths = malloc(sizeof(loader_naming_path) * size);

	if (paths == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader load from configuration invalid paths allocation");

		configuration_clear(config);

		return 1;
	}

	scripts_array = value_to_list(scripts);

	for (index = 0; index < size; ++index)
	{
		const char * str = value_to_string(scripts_array[index]);

		strncpy(paths[index], str, value_type_size(scripts_array[index]));

		value_destroy(scripts_array[index]);
	}

	if (loader_load_from_file(value_to_string(tag), paths, size) != 0)
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

int loader_get_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args)
{
	if (map != NULL && key != NULL && val != NULL && args != NULL)
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
		hash_map_cb_iterate_args args;

		struct loader_get_iterator_args_type get_args;

		get_args.name = name;
		get_args.obj = NULL;

		args = (loader_get_iterator_args)&get_args;

		hash_map_iterate(l->impl_map, &loader_get_cb_iterate, args);

		if (get_args.obj != NULL)
		{
			return (loader_data)get_args.obj;
		}
	}

	return NULL;
}

int loader_inspect_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args)
{
	if (map != NULL && key != NULL && val != NULL && args != NULL)
	{
		struct loader_inspect_cb_iterate_type * inspect_iterator = args;

		loader_impl impl = val;

		context ctx = loader_impl_context(impl);

		const char * ctx_name = context_name(ctx);

		size_t ctx_name_size = strlen(ctx_name);

		scope sp = context_scope(ctx);

		size_t sp_buffer_size = 0;

		char * sp_buffer = scope_dump(sp, &sp_buffer_size);

		if (sp_buffer == NULL)
		{
			return 0;
		}

		if (inspect_iterator->buffer == NULL)
		{
			inspect_iterator->buffer = malloc((sp_buffer_size + ctx_name_size + 3) * sizeof(char));

			if (inspect_iterator->buffer == NULL)
			{
				free(sp_buffer);

				return 1;
			}
		}
		else
		{
			char * new_buffer = realloc(inspect_iterator->buffer, (inspect_iterator->size + sp_buffer_size + ctx_name_size + 3) * sizeof(char));

			if (new_buffer == NULL)
			{
				free(inspect_iterator->buffer);
				free(sp_buffer);

				inspect_iterator->buffer = NULL;
				inspect_iterator->size = 0;

				return 1;
			}
			else
			{
				inspect_iterator->buffer = new_buffer;
			}
		}

		inspect_iterator->buffer[inspect_iterator->size++] = '@';

		memcpy(&inspect_iterator->buffer[inspect_iterator->size], ctx_name, ctx_name_size);

		inspect_iterator->size += ctx_name_size;

		inspect_iterator->buffer[inspect_iterator->size++] = '\n';

		memcpy(&inspect_iterator->buffer[inspect_iterator->size], sp_buffer, sp_buffer_size - 1);

		inspect_iterator->size += sp_buffer_size - 1;

		inspect_iterator->buffer[inspect_iterator->size++] = '\n';

		free(sp_buffer);

		return 0;
	}

	return 1;
}

char * loader_inspect(size_t * size)
{
	loader l = loader_singleton();

	if (l->impl_map != NULL)
	{
		struct loader_inspect_cb_iterate_type inspect_iterator;

		inspect_iterator.buffer = NULL;
		inspect_iterator.size = 0;

		hash_map_iterate(l->impl_map, &loader_inspect_cb_iterate, &inspect_iterator);

		if (inspect_iterator.buffer != NULL)
		{
			inspect_iterator.buffer[inspect_iterator.size - 1] = '\0';

			log_write("metacall", LOG_LEVEL_DEBUG, "Loader inspection [\n%s\n]", inspect_iterator.buffer);

			*size = inspect_iterator.size;

			return inspect_iterator.buffer;
		}
	}

	return NULL;
}

int loader_unload_impl_map_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args)
{
	if (map != NULL && key != NULL && val != NULL && args == NULL)
	{
		loader_impl impl = val;

		loader_impl_destroy(impl);

		return 0;
	}

	return 1;
}

int loader_unload()
{
	loader l = loader_singleton();

	if (l->impl_map != NULL)
	{
		hash_map_iterate(l->impl_map, &loader_unload_impl_map_cb_iterate, NULL);

		if (hash_map_clear(l->impl_map) != 0)
		{
			#ifdef LOADER_LAZY
				log_write("metacall", LOG_LEVEL_DEBUG, "Loader lazy destruction");

				loader_destroy();
			#endif

			return 1;
		}
	}

	#ifdef LOADER_LAZY
		log_write("metacall", LOG_LEVEL_DEBUG, "Loader lazy destruction");

		loader_destroy();
	#endif

	return 0;
}

void loader_destroy()
{
	loader l = loader_singleton();

	if (l->impl_map != NULL)
	{
		hash_map_destroy(l->impl_map);

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
		"Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef LOADER_STATIC_DEFINE
			"Compiled as static library type\n"
		#else
			"Compiled as shared library type\n"
		#endif

		#ifdef LOADER_LAZY
			"Compiled with lazy initialization and destruction"
		#else
			"Compiled with explicit initialization and destruction"
		#endif

		"\n";

	return loader_info;
}
