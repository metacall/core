/*
 *	Loader Library by Parra Studios
 *	A plugin for loading TypeScript code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <ts_loader/ts_loader_impl.h>
#include <node_loader/node_loader_bootstrap.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>
#include <loader/loader_path.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

#include <log/log.h>

#include <metacall/metacall.h>

#include <string.h>

int ts_loader_impl_initialize_types(loader_impl impl)
{
	static struct
	{
		type_id id;
		const char * name;
	}
	type_id_name_pair[] =
	{
		{ TYPE_BOOL,		"boolean"					},
		{ TYPE_DOUBLE,		"number"					},
		{ TYPE_STRING,		"string"					},
		{ TYPE_NULL,		"null"						},
		{ TYPE_MAP,			"Record<any, any>"			},
		{ TYPE_ARRAY,		"any[]"						},
		{ TYPE_FUNCTION,	"(...args: any[]) => any"	}
	};

	size_t index, size = sizeof(type_id_name_pair) / sizeof(type_id_name_pair[0]);

	for (index = 0; index < size; ++index)
	{
		type t = type_create(type_id_name_pair[index].id, type_id_name_pair[index].name, NULL, NULL);

		if (t != NULL)
		{
			if (loader_impl_type_define(impl, type_name(t), t) != 0)
			{
				type_destroy(t);

				return 1;
			}
		}
	}

	return 0;
}

loader_impl_data ts_loader_impl_initialize(loader_impl impl, configuration config, loader_host host)
{
	static const char bootstrap_file_str[] = "bootstrap.ts";
	node_impl_path bootstrap_path_str = { 0 };
	size_t bootstrap_path_str_size = 0;
	const char * paths[1];
	void * ts_impl = NULL;

	loader_copy(host);

	/* Get the boostrap path */
	if (node_loader_impl_bootstrap_path(bootstrap_file_str, config, bootstrap_path_str, &bootstrap_path_str_size) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "LOADER_LIBRARY_PATH not defined, bootstrap.ts cannot be found");

		return NULL;
	}

	/* Initialize TypeScript types */
	if (ts_loader_impl_initialize_types(impl) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "TypeScript Loader failed to initialize the types");

		return NULL;
	}

	/* Load TypeScript bootstrap */
	paths[0] = bootstrap_path_str;

	if (metacall_load_from_file("node", paths, 1, &ts_impl) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "TypeScript bootstrap.ts cannot be loaded");

		return NULL;
	}

	/* Initialize bootstrap */
	void * ret = metacallhv_s(ts_impl, "initialize", metacall_null_args, 0);

	// TODO: Do something with the value like error handling?

	metacall_value_destroy(ret);

	return (loader_impl_data)ts_impl;
}

int ts_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	void * ts_impl = (void *)loader_impl_get(impl);
	void * args[1];

	args[0] = metacall_value_create_string(path, strlen(path));

	void * ret = metacallhv_s(ts_impl, "execution_path", args, 1);

	metacall_value_destroy(args[0]);

	// TODO: Do something with the value like error handling?

	metacall_value_destroy(ret);

	return 0;
}

loader_handle ts_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	void * ts_impl = (void *)loader_impl_get(impl);
	void * args[1] = { metacall_value_create_array(NULL, size) };
	size_t iterator;

	if (args[0] == NULL)
	{
		// TODO: Error handling
		return NULL;
	}

	void ** args_array = metacall_value_to_array(args[0]);

	for (iterator = 0; iterator < size; ++iterator)
	{
		args_array[iterator] = metacall_value_create_string(paths[iterator], strlen(paths[iterator]));
	}

	void * ret = metacallhv_s(ts_impl, "load_from_file", args, 1);

	metacall_value_destroy(args[0]);

	return (loader_handle)ret;
}

loader_handle ts_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size)
{
	void * ts_impl = (void *)loader_impl_get(impl);
	void * args[2];

	args[0] = metacall_value_create_string(name, strlen(name));
	args[1] = metacall_value_create_string(buffer, size - 1);

	void * ret = metacallhv_s(ts_impl, "load_from_memory", args, 2);

	metacall_value_destroy(args[0]);

	return (loader_handle)ret;
}

loader_handle ts_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	void * ts_impl = (void *)loader_impl_get(impl);

	void * args[1];

	args[0] = metacall_value_create_string(path, strlen(path));

	void * ret = metacallhv_s(ts_impl, "load_from_package", args, 1);

	metacall_value_destroy(args[0]);

	return (loader_handle)ret;
}

int ts_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	void * ts_impl = (void *)loader_impl_get(impl);

	void * args[1];

	args[0] = (void *)handle;

	void * ret = metacallhv_s(ts_impl, "clear", args, 1);

	// TODO: Do something with the value like error handling?

	metacall_value_destroy(ret);

	metacall_value_destroy((void *)handle);

	return 0;
}

int ts_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	void * ts_impl = (void *)loader_impl_get(impl);

	void * args[1];

	args[0] = (void *)handle;

	void * ret = metacallhv_s(ts_impl, "discover", args, 1);

	// TODO: Implement introspection
	(void)ctx;







	metacall_value_destroy(ret);

	return 0;
}

int ts_loader_impl_destroy(loader_impl impl)
{
	void * ts_impl = (void *)loader_impl_get(impl);

	if (ts_impl == NULL)
	{
		return 1;
	}

	/* Destroy children loaders */
	loader_unload_children();

	return metacall_clear(ts_impl);
}
