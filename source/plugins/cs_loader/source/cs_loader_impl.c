/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading net code at run-time into a process.
 *
 */

#include <cs_loader/cs_loader_impl.h>

#include <loader/loader_impl.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

#include <log/log.h>

#include <stdlib.h>

#include <cs_loader/simple_netcore.h>

typedef struct {
	netcore_handle handle;
	reflect_function * func;
} cs_function;



int function_cs_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

function_return function_cs_interface_invoke(function func, function_impl impl, function_args args)
{
	(void)func;
	(void)impl;
	(void)args;

	cs_function * cs_f = (cs_function*)impl;

	simple_netcore_invoke(cs_f->handle, cs_f->func->name);

	return NULL;
}

void function_cs_interface_destroy(function func, function_impl impl)
{
	(void)func;
	(void)impl;
}

function_interface function_cs_singleton(void)
{
	static struct function_interface_type cs_interface =
	{
		&function_cs_interface_create,
		&function_cs_interface_invoke,
		&function_cs_interface_destroy
	};

	return &cs_interface;
}

int cs_loader_impl_initialize_types(loader_impl impl)
{
	/* TODO: move this to loader_impl by passing the structure and loader_impl_derived callback */

	(void)impl;

	/* TODO: Load your custom types if required */

	/*
	static struct
	{
		type_id id;
		const char * name;
	}
	type_id_name_pair[] =
	{
		{ TYPE_BOOL, "bool" },
		{ TYPE_INT, "int" },
		{ TYPE_LONG, "long" },
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
	*/

	return 0;
}

loader_impl_data cs_loader_impl_initialize(loader_impl impl)
{
	/* TODO: Initialize runtime, call to type initialization if need */

	(void)impl;

	return (loader_impl_data)simple_netcore_create();
}

int cs_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	/* TODO: Insert a path into the runtime */

	(void)impl;
	(void)path;

	return 0;
}

loader_handle cs_loader_impl_load(loader_impl impl, const loader_naming_path path, loader_naming_name name)
{
	/* TODO: Load a new script into a loader_handle by path and name (just that, not inspection / reflection need */

	(void)impl;
	(void)path;
	(void)name;

	netcore_handle nhandle = (netcore_handle)loader_impl_get(impl);

	simple_netcore_load_script(nhandle, path, name);

	return NULL;
}

int cs_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	/* TODO: Clear a script (defined by handle) */

	(void)impl;
	(void)handle;

	return 0;
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
	reflect_function * functions;

	functions = simple_netcore_get_functions(nhandle, &function_count);

	function f = NULL;

	for (size_t i = 0; i < function_count; ++i)
	{
		cs_function * cs_f = (cs_function*)malloc(sizeof(cs_function));

		cs_f->func = &functions[i];
		cs_f->handle = nhandle;

		f = function_create(functions[i].name, functions[i].param_count, cs_f, &function_cs_singleton);

		if (functions[i].param_count > 0) {

			signature s = function_signature(f);

			for (size_t j = 0; j < functions[i].param_count; ++j)
			{
				signature_set(s, j, functions[i].pars[j].name, (type)&functions[i].pars[j].type);
			}

			signature_set_return(s, (type)&functions[i].return_type);
		}

		scope_define(sp, functions[i].name, f);
	}

	return 0;
}

int cs_loader_impl_destroy(loader_impl impl)
{
	/* TODO: Destroy runtime */

	(void)impl;

	netcore_handle nhandle = (netcore_handle)loader_impl_get(impl);

	simple_netcore_destroy(nhandle);

	return 0;
}
