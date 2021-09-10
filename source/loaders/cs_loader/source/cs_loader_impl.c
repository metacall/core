/*
 *	Loader Library by Parra Studios
 *	A plugin for loading net code at run-time into a process.
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

#include <cs_loader/cs_loader_impl.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>

#include <log/log.h>

#include <configuration/configuration.h>

#include <cs_loader/simple_netcore.h>
#include <stdlib.h>

// TODO: This design has to change, it needs an AppDomain per list of functions (https://github.com/metacall/core/issues/123)

typedef struct
{
	netcore_handle handle;
	reflect_function *func;
} cs_function;

int function_cs_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

function_return function_cs_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	(void)func;
	(void)size; /* TODO: Assert size and param_count are equal, varidic not allowed in C# yet */

	cs_function *cs_f = (cs_function *)impl;
	execution_result *result;

	if (cs_f->func->param_count == 0)
	{
		result = simple_netcore_invoke(cs_f->handle, cs_f->func->name);
	}
	else
	{
		// TODO: Do not hardcode this, take the information from the function and (probably) preload the list of arguments
		parameters params[10];

		for (int i = 0; i < cs_f->func->param_count; ++i)
		{
			params[i].ptr = args[i];
			params[i].type = cs_f->func->pars[i].type;
		}

		result = simple_netcore_invoke_with_params(cs_f->handle, cs_f->func->name, params);
	}

	value v = NULL;

	if (result->ptr != NULL)
	{
		switch (cs_f->func->return_type)
		{
			case TYPE_BOOL: {
				v = value_create_bool(*(boolean *)result->ptr);
				break;
			}

			case TYPE_CHAR: {
				v = value_create_bool(*(char *)result->ptr);
				break;
			}

			case TYPE_SHORT: {
				v = value_create_short(*(short *)result->ptr);
				break;
			}

			case TYPE_INT: {
				v = value_create_int(*(int *)result->ptr);
				break;
			}

			case TYPE_LONG: {
				v = value_create_long(*(long *)result->ptr);
				break;
			}

			case TYPE_FLOAT: {
				v = value_create_float(*(float *)result->ptr);
				break;
			}

			case TYPE_DOUBLE: {
				v = value_create_double(*(double *)result->ptr);
				break;
			}

			case TYPE_STRING: {
				v = value_create_string((const char *)result->ptr, strlen((const char *)result->ptr));
				break;
			}
		}
	}

	simple_netcore_destroy_execution_result(cs_f->handle, result);

	return v;
}

function_return function_cs_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
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

void function_cs_interface_destroy(function func, function_impl impl)
{
	(void)func;
	(void)impl;
}

function_interface function_cs_singleton(void)
{
	static struct function_interface_type cs_interface = {
		&function_cs_interface_create,
		&function_cs_interface_invoke,
		&function_cs_interface_await,
		&function_cs_interface_destroy
	};

	return &cs_interface;
}

int cs_loader_impl_initialize_types(loader_impl impl)
{
	/* TODO: move this to loader_impl by passing the structure and loader_impl_derived callback */

	static struct
	{
		type_id id;
		const char *name;
	} type_id_name_pair[] = {
		{ TYPE_BOOL, "bool" },
		{ TYPE_CHAR, "char" },
		{ TYPE_SHORT, "short" },
		{ TYPE_INT, "int" },
		{ TYPE_LONG, "long" },
		{ TYPE_FLOAT, "float" },
		{ TYPE_DOUBLE, "double" },
		{ TYPE_STRING, "string" }
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

loader_impl_data cs_loader_impl_initialize(loader_impl impl, configuration config)
{
	char *dotnet_root = NULL;
	char *dotnet_loader_assembly_path = NULL;
	value dotnet_root_value = NULL;
	value dotnet_loader_assembly_path_value = NULL;
	netcore_handle nhandle = NULL;

	if (cs_loader_impl_initialize_types(impl) != 0)
	{
		return NULL;
	}

	if (config != NULL)
	{
		dotnet_root_value = configuration_value(config, "dotnet_root");
		dotnet_loader_assembly_path_value = configuration_value(config, "dotnet_loader_assembly_path");

		if (dotnet_root_value != NULL)
		{
			dotnet_root = value_to_string(dotnet_root_value);
		}

		if (dotnet_loader_assembly_path_value != NULL)
		{
			dotnet_loader_assembly_path = value_to_string(dotnet_loader_assembly_path_value);
		}
	}

	nhandle = simple_netcore_create(dotnet_root, dotnet_loader_assembly_path);

	if (nhandle != NULL)
	{
		loader_initialization_register(impl);
	}

	return (loader_impl_data)nhandle;
}

int cs_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	netcore_handle nhandle = (netcore_handle)loader_impl_get(impl);

	return simple_netcore_execution_path(nhandle, (char *)path);
}

loader_handle cs_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	netcore_handle nhandle = (netcore_handle)loader_impl_get(impl);

	char *files[MAX_FILES];

	size_t i;

	for (i = 0; i < size; ++i)
	{
		files[i] = (char *)paths[i];
	}

	if (simple_netcore_load_script_from_files(nhandle, (const char **)files, size) != 0)
	{
		return NULL;
	}

	return (loader_handle)impl;
}

loader_handle cs_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	netcore_handle nhandle = (netcore_handle)loader_impl_get(impl);

	if (simple_netcore_load_script_from_assembly(nhandle, path) != 0)
	{
		return NULL;
	}

	return (loader_handle)impl;
}

loader_handle cs_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char *buffer, size_t size)
{
	(void)name;

	netcore_handle nhandle = (netcore_handle)loader_impl_get(impl);

	if (simple_netcore_load_script_from_memory(nhandle, buffer, size) != 0)
	{
		return NULL;
	}

	return (loader_handle)impl;
}

int cs_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	/* TODO: Clear a script (defined by handle) */

	(void)impl;
	(void)handle;

	return 0;
}

static const char *cs_loader_impl_discover_type(short id)
{
	/* TODO:
		This function is needed because of a bad implementation
		which breaks the original design, reimplementing its own type id system
		and function representation system without respecting original data types.
		This plugin has to be reestructured and reimplemented if necessary to
		keep the whole system consistent and to fit the original design.
		Remove this function in the future.
	*/

	static struct
	{
		short id;
		const char *name;
	} type_id_name_pair[] = {
		{ TYPE_BOOL, "bool" },
		{ TYPE_CHAR, "char" },
		{ TYPE_INT, "int" },
		{ TYPE_LONG, "long" },
		{ TYPE_FLOAT, "float" },
		{ TYPE_DOUBLE, "double" },
		{ TYPE_STRING, "string" }
	};

	size_t index, size = sizeof(type_id_name_pair) / sizeof(type_id_name_pair[0]);

	for (index = 0; index < size; ++index)
	{
		if (type_id_name_pair[index].id == id)
		{
			return type_id_name_pair[index].name;
		}
	}

	return "(NULL)";
}

int cs_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	/* TODO: Discover handle (script) and insert metadata information of the script into context */

	(void)impl;
	(void)handle;
	(void)ctx;

	netcore_handle nhandle = (netcore_handle)loader_impl_get(impl);

	scope sp = context_scope(ctx);

	int function_count = 0;
	reflect_function *functions;

	functions = simple_netcore_get_functions(nhandle, &function_count);

	function f = NULL;

	for (int i = 0; i < function_count; ++i)
	{
		cs_function *cs_f = (cs_function *)malloc(sizeof(cs_function));

		cs_f->func = &functions[i];
		cs_f->handle = nhandle;

		f = function_create(functions[i].name, functions[i].param_count, cs_f, &function_cs_singleton);

		if (f != NULL)
		{
			signature s = function_signature(f);

			signature_set_return(s, loader_impl_type(impl, cs_loader_impl_discover_type(functions[i].return_type)));

			for (int j = 0; j < functions[i].param_count; ++j)
			{
				type t = loader_impl_type(impl, cs_loader_impl_discover_type(functions[i].pars[j].type));

				signature_set(s, j, functions[i].pars[j].name, t);
			}
		}

		scope_define(sp, functions[i].name, value_create_function(f));
	}

	return 0;
}

int cs_loader_impl_destroy(loader_impl impl)
{
	netcore_handle nhandle = (netcore_handle)loader_impl_get(impl);

	/* Destroy children loaders */
	loader_unload_children(impl, 0);

	simple_netcore_destroy(nhandle);

	return 0;
}
