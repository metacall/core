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
#include <node_loader/node_loader_impl.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>
#include <loader/loader_path.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

#include <log/log.h>

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
	loader_impl_data node_loader_impl;

	/* Detect if bootstrap script has been defined */
	static const char bootstrap_script_key[] = "bootstrap_script";
	value bootstrap_value = NULL;
	configuration ts_config = NULL;

	if (config == NULL)
	{
		ts_config = configuration_create("ts_loader", NULL, NULL, NULL);

		if (ts_config == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "TypeScript Loader failed to create the configuration");

			return NULL;
		}
		else
		{
			config = ts_config;
		}
	}

	if (configuration_value(config, bootstrap_script_key) == NULL)
	{
		/* Define a default bootstrap script file name */
		static const char bootstrap_script[] = "bootstrap.ts";

		bootstrap_value = value_create_string(bootstrap_script, sizeof(bootstrap_script) - 1);

		if (bootstrap_value == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "TypeScript Loader failed to allocate the bootstrap value");

			return NULL;
		}

		if (configuration_define(config, bootstrap_script_key, bootstrap_value) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "TypeScript Loader failed to define the bootstrap value in the configuration");

			value_type_destroy(bootstrap_value);

			return NULL;
		}
	}

	/* Initialize the Node Loader */
	node_loader_impl = node_loader_impl_initialize(impl, config, host);

	/* Clear the bootstrap default value */
	if (bootstrap_value != NULL)
	{
		value_type_destroy(bootstrap_value);

		if (configuration_undefine(config, bootstrap_script_key) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "TypeScript Loader failed to undefine the bootstrap value from the configuration");

			if (node_loader_impl_destroy(impl) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "TypeScript Loader failed to destroy Node Loader");
			}

			/* Clear the configuration if any */
			if (ts_config != NULL)
			{
				if (configuration_clear(ts_config) != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "TypeScript Loader failed clear the configuration");
				}
			}

			return NULL;
		}
	}

	/* Clear the configuration if any */
	if (ts_config != NULL)
	{
		if (configuration_clear(ts_config) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "TypeScript Loader failed clear the configuration");
		}
	}

	if (node_loader_impl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "TypeScript Loader failed to initialize Node Loader");

		return NULL;
	}

	/* Initialize TypeScript types */
	if (ts_loader_impl_initialize_types(impl) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "TypeScript Loader failed to initialize the types");

		if (node_loader_impl_destroy(impl) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "TypeScript Loader failed to destroy Node Loader");
		}

		return NULL;
	}

	return node_loader_impl;
}

int ts_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	return node_loader_impl_execution_path(impl, path);
}

loader_handle ts_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	return node_loader_impl_load_from_file(impl, paths, size);
}

loader_handle ts_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size)
{
	return node_loader_impl_load_from_memory(impl, name, buffer, size);
}

loader_handle ts_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	return node_loader_impl_load_from_package(impl, path);
}

int ts_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	return node_loader_impl_clear(impl, handle);
}

int ts_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	return node_loader_impl_discover(impl, handle, ctx);
}

int ts_loader_impl_destroy(loader_impl impl)
{
	/* Destroy children loaders */
	loader_unload_children();

	return node_loader_impl_destroy(impl);
}
