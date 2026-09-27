/*
 *	Loader Library by Parra Studios
 *	A plugin for loading net code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

typedef struct
{
	netcore_handle handle;
	char name[100];
} cs_class;

typedef struct
{
	netcore_handle handle;
	void *managed_object;
} cs_object;

static value cs_loader_impl_execution_result_to_value(netcore_handle handle, execution_result *result)
{
	value v = NULL;

	if (result == NULL)
	{
		return NULL;
	}

	if (result->failed != 0)
	{
		simple_netcore_destroy_execution_result(
			handle,
			result);

		return NULL;
	}

	if (result->ptr == NULL)
	{
		v = value_create_null();
	}
	else
	{
		switch (result->type)
		{
			case TYPE_BOOL:
				v = value_create_bool(
					*(boolean *)result->ptr);
				break;

			case TYPE_CHAR:
				v = value_create_char(
					*(char *)result->ptr);
				break;

			case TYPE_SHORT:
				v = value_create_short(
					*(short *)result->ptr);
				break;

			case TYPE_INT:
				v = value_create_int(
					*(int *)result->ptr);
				break;

			case TYPE_LONG:
				v = value_create_long(
					*(long *)result->ptr);
				break;

			case TYPE_FLOAT:
				v = value_create_float(
					*(float *)result->ptr);
				break;

			case TYPE_DOUBLE:
				v = value_create_double(
					*(double *)result->ptr);
				break;

			case TYPE_STRING:
				v = value_create_string(
					(const char *)result->ptr,
					strlen((const char *)result->ptr));
				break;

			case TYPE_PTR:
				v = value_create_ptr(result->ptr);
				break;

			default:
				v = value_create_null();
				break;
		}
	}

	simple_netcore_destroy_execution_result(handle, result);

	return v;
}

static void cs_loader_impl_parameters(parameters *params, void **args, size_t size)
{
	for (size_t i = 0; i < size; ++i)
	{
		params[i].ptr = args[i];
		params[i].type =
			(short)value_type_id((value)args[i]);
	}
}

static void cs_loader_impl_parameter(parameters *param, value v)
{
	param->type = (short)value_type_id(v);
	param->ptr = value_data(v);
}

static int cs_object_interface_create(object obj, object_impl impl)
{
	(void)obj;
	(void)impl;

	return 0;
}

static value cs_object_interface_get(object obj, object_impl impl, struct accessor_type *accessor)
{
	(void)obj;

	cs_object *cs_o = (cs_object *)impl;

	return cs_loader_impl_execution_result_to_value(
		cs_o->handle,
		simple_netcore_get_object_attribute(
			cs_o->handle,
			cs_o->managed_object,
			attribute_name(accessor->data.attr)));
}

static int cs_object_interface_set(object obj, object_impl impl, struct accessor_type *accessor, value v)
{
	(void)obj;

	cs_object *cs_o = (cs_object *)impl;
	parameters param;

	cs_loader_impl_parameter(&param, v);

	return simple_netcore_set_object_attribute(
		cs_o->handle,
		cs_o->managed_object,
		attribute_name(accessor->data.attr),
		&param);
}

static value cs_object_interface_method_invoke(object obj, object_impl impl, method m, object_args args, size_t size)
{
	(void)obj;

	cs_object *cs_o = (cs_object *)impl;
	parameters params[10];

	if (size > 10)
	{
		return NULL;
	}

	cs_loader_impl_parameters(
		params,
		args,
		size);

	return cs_loader_impl_execution_result_to_value(
		cs_o->handle,
		simple_netcore_invoke_object(
			cs_o->handle,
			cs_o->managed_object,
			method_name(m),
			params,
			size));
}

static value cs_object_interface_method_await(object obj, object_impl impl, method m, object_args args, size_t size, object_resolve_callback resolve, object_reject_callback reject, void *ctx)
{
	(void)obj;
	(void)impl;
	(void)m;
	(void)args;
	(void)size;
	(void)resolve;
	(void)reject;
	(void)ctx;

	return NULL;
}

static int cs_object_interface_destructor(object obj, object_impl impl)
{
	(void)obj;

	cs_object *cs_o = (cs_object *)impl;

	if (cs_o != NULL &&
		cs_o->managed_object != NULL)
	{
		simple_netcore_destroy_object(
			cs_o->handle,
			cs_o->managed_object);

		cs_o->managed_object = NULL;
	}

	return 0;
}

static void cs_object_interface_destroy(object obj, object_impl impl)
{
	(void)obj;

	free(impl);
}

static object_interface cs_object_interface_singleton(void)
{
	static struct object_interface_type interface = {
		&cs_object_interface_create,
		&cs_object_interface_get,
		&cs_object_interface_set,
		&cs_object_interface_method_invoke,
		&cs_object_interface_method_await,
		&cs_object_interface_destructor,
		&cs_object_interface_destroy
	};

	return &interface;
}

static object cs_class_interface_constructor(klass cls, class_impl impl, const char *name, constructor ctor, class_args args, size_t argc)
{
	(void)ctor;

	cs_class *cs_c = (cs_class *)impl;
	parameters params[10];

	if (argc > 10)
	{
		return NULL;
	}

	cs_loader_impl_parameters(params, args, argc);

	void *managed_object =
		simple_netcore_create_object(cs_c->handle, cs_c->name, params, argc);

	if (managed_object == NULL)
	{
		return NULL;
	}

	cs_object *cs_o =
		(cs_object *)calloc(1, sizeof(cs_object));

	if (cs_o == NULL)
	{
		simple_netcore_destroy_object(cs_c->handle, managed_object);

		return NULL;
	}

	cs_o->handle = cs_c->handle;
	cs_o->managed_object = managed_object;

	object obj = object_create(name, ACCESSOR_TYPE_STATIC, cs_o, &cs_object_interface_singleton, cls);

	if (obj == NULL)
	{
		simple_netcore_destroy_object(cs_c->handle, managed_object);

		free(cs_o);
	}

	return obj;
}

static value cs_class_interface_static_get(
	klass cls, class_impl impl, struct accessor_type *accessor)
{
	(void)cls;

	cs_class *cs_c = (cs_class *)impl;

	return cs_loader_impl_execution_result_to_value(cs_c->handle, simple_netcore_get_static_attribute(cs_c->handle, cs_c->name, attribute_name(accessor->data.attr)));
}

static int cs_class_interface_static_set(klass cls, class_impl impl, struct accessor_type *accessor, value v)
{
	(void)cls;

	cs_class *cs_c = (cs_class *)impl;
	parameters param;

	cs_loader_impl_parameter(&param, v);

	return simple_netcore_set_static_attribute(cs_c->handle, cs_c->name, attribute_name(accessor->data.attr), &param);
}

static value cs_class_interface_static_invoke(klass cls, class_impl impl, method m, class_args args, size_t size)
{
	(void)cls;

	cs_class *cs_c = (cs_class *)impl;
	parameters params[10];

	if (size > 10)
	{
		return NULL;
	}

	cs_loader_impl_parameters(params, args, size);

	return cs_loader_impl_execution_result_to_value(cs_c->handle, simple_netcore_invoke_static(cs_c->handle, cs_c->name, method_name(m), params, size));
}

static value cs_class_interface_static_await(klass cls, class_impl impl, method m, class_args args, size_t size, class_resolve_callback resolve, class_reject_callback reject, void *ctx)
{
	(void)cls;
	(void)impl;
	(void)m;
	(void)args;
	(void)size;
	(void)resolve;
	(void)reject;
	(void)ctx;

	return NULL;
}

static void cs_class_interface_destroy(klass cls, class_impl impl)
{
	(void)cls;

	free(impl);
}

static int cs_class_interface_create(klass cls, class_impl impl)
{
	(void)cls;
	(void)impl;

	return 0;
}

static class_interface cs_class_interface_singleton(void)
{
	static struct class_interface_type interface = {
		&cs_class_interface_create,
		&cs_class_interface_constructor,
		&cs_class_interface_static_get,
		&cs_class_interface_static_set,
		&cs_class_interface_static_invoke,
		&cs_class_interface_static_await,
		&cs_class_interface_destroy
	};

	return &interface;
}

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
	cs_function *cs_f = (cs_function *)impl;

	(void)func;

	free(cs_f);
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
		dotnet_root_value = configuration_value_type(config, "dotnet_root", TYPE_STRING);
		dotnet_loader_assembly_path_value = configuration_value_type(config, "dotnet_loader_assembly_path", TYPE_STRING);

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

int cs_loader_impl_execution_path(loader_impl impl, const loader_path path)
{
	netcore_handle nhandle = (netcore_handle)loader_impl_get(impl);

	return simple_netcore_execution_path(nhandle, (char *)path);
}

loader_handle cs_loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size, void *data)
{
	netcore_handle nhandle = (netcore_handle)loader_impl_get(impl);
	char *files[MAX_FILES];
	size_t i;

	(void)data;

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

loader_handle cs_loader_impl_load_from_package(loader_impl impl, const loader_path path, void *data)
{
	netcore_handle nhandle = (netcore_handle)loader_impl_get(impl);

	(void)data;

	if (simple_netcore_load_script_from_assembly(nhandle, path) != 0)
	{
		return NULL;
	}

	return (loader_handle)impl;
}

loader_handle cs_loader_impl_load_from_memory(loader_impl impl, const loader_name name, const char *buffer, size_t size, void *data)
{
	netcore_handle nhandle = (netcore_handle)loader_impl_get(impl);

	(void)name;
	(void)data;

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

static int cs_loader_impl_discover_classes(loader_impl impl, netcore_handle nhandle, scope sp)
{
	int class_count = 0;
	int constructor_count = 0;
	int method_count = 0;
	int attribute_count = 0;

	reflect_class *classes = simple_netcore_get_classes(nhandle, &class_count);

	reflect_constructor *constructors = simple_netcore_get_constructors(nhandle, &constructor_count);

	reflect_method *methods = simple_netcore_get_methods(nhandle, &method_count);

	reflect_attribute *attributes = simple_netcore_get_attributes(nhandle, &attribute_count);

	for (int i = 0; i < class_count; ++i)
	{
		cs_class *cs_c = (cs_class *)calloc(1, sizeof(cs_class));

		if (cs_c == NULL)
		{
			return 1;
		}

		strncpy(cs_c->name, classes[i].name, sizeof(cs_c->name) - 1);

		cs_c->handle = nhandle;

		klass c = class_create(cs_c->name, ACCESSOR_TYPE_STATIC, cs_c, &cs_class_interface_singleton);

		if (c == NULL)
		{
			free(cs_c);
			continue;
		}

		for (int j = 0; j < constructor_count; ++j)
		{
			if (strcmp(constructors[j].class_name, cs_c->name) != 0)
			{
				continue;
			}

			constructor ctor = constructor_create((size_t)constructors[j].param_count, VISIBILITY_PUBLIC);

			if (ctor == NULL)
			{
				continue;
			}

			for (int k = 0; k < constructors[j].param_count; ++k)
			{
				type t = loader_impl_type(impl, cs_loader_impl_discover_type(constructors[j].pars[k].type));

				constructor_set(ctor, (size_t)k, constructors[j].pars[k].name, t);
			}

			class_register_constructor(c, ctor);
		}

		for (int j = 0; j < method_count; ++j)
		{
			if (strcmp(methods[j].class_name, cs_c->name) != 0)
			{
				continue;
			}

			method m = method_create(c, methods[j].name, (size_t)methods[j].param_count, NULL, VISIBILITY_PUBLIC, SYNCHRONOUS, NULL);

			if (m == NULL)
			{
				continue;
			}

			signature s = method_signature(m);

			type return_type = loader_impl_type(impl, cs_loader_impl_discover_type(methods[j].return_type));

			signature_set_return(s, return_type);

			for (int k = 0; k < methods[j].param_count; ++k)
			{
				type t = loader_impl_type(impl, cs_loader_impl_discover_type(methods[j].pars[k].type));

				signature_set(s, k, methods[j].pars[k].name, t);
			}

			if (methods[j].is_static != 0)
			{
				class_register_static_method(c, m);
			}
			else
			{
				class_register_method(c, m);
			}
		}

		for (int j = 0; j < attribute_count; ++j)
		{
			if (strcmp(attributes[j].class_name, cs_c->name) != 0)
			{
				continue;
			}

			type t = loader_impl_type(impl, cs_loader_impl_discover_type(attributes[j].type));

			attribute attr = attribute_create(c, attributes[j].name, t, NULL, VISIBILITY_PUBLIC, NULL);

			if (attr == NULL)
			{
				continue;
			}

			if (attributes[j].is_static != 0)
			{
				class_register_static_attribute(c, attr);
			}
			else
			{
				class_register_attribute(c, attr);
			}
		}

		value v = value_create_class(c);

		if (v == NULL)
		{
			class_destroy(c);
			return 1;
		}

		if (scope_define(sp, class_name(c), v) != 0)
		{
			/*
			 * Preserve the legacy flattened function when a class
			 * has the same exported name.
			 */
			value_type_destroy(v);
			continue;
		}
	}

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

		value v = value_create_function(f);

		if (scope_define(sp, function_name(f), v) != 0)
		{
			value_type_destroy(v);
			return 1;
		}
	}

	if (cs_loader_impl_discover_classes(impl, nhandle, sp) != 0)
	{
		return 1;
	}

	return 0;
}

int cs_loader_impl_destroy(loader_impl impl)
{
	netcore_handle nhandle = (netcore_handle)loader_impl_get(impl);

	/* Destroy children loaders */
	loader_unload_children(impl);

	simple_netcore_destroy(nhandle);

	return 0;
}
