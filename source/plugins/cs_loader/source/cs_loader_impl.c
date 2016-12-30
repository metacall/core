/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
	parameters params[0xF];

	cs_function * cs_f = (cs_function*)impl;

	signature s = function_signature(func);

	const size_t args_size = signature_count(s);

	//type ret_type = signature_get_return(s);

	size_t args_count;

	for (args_count = 0; args_count < args_size; ++args_count)
	{
		params[args_count].type = (short)cs_f->func->pars[args_count].type;
		params[args_count].ptr = args[args_count];
	}
	if (args_size > 0) {
		simple_netcore_invoke(cs_f->handle, cs_f->func->name, params, args_size);
	}
	else
	{
		simple_netcore_invoke(cs_f->handle, cs_f->func->name, NULL, 0);
	}

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


loader_handle cs_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const loader_naming_extension ext, const char * code, size_t size) {
	(void)impl;
	(void)name;
	(void)ext;
	(void)code;
	(void)size;

	return NULL;
}


loader_handle cs_loader_impl_load_from_file(loader_impl impl, const loader_naming_path path, const loader_naming_name name)
{
	/* TODO: Load a new script into a loader_handle by path and name (just that, not inspection / reflection need */

	(void)impl;
	(void)path;
	(void)name;

	netcore_handle nhandle = (netcore_handle)loader_impl_get(impl);

	simple_netcore_load_script(nhandle, path, name);

	return nhandle;
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
