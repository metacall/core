/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading ruby code at run-time into a process.
 *
 */

#include <rb_loader/rb_loader_impl.h>

#include <loader/loader_impl.h>

#include <reflect/type.h>
#include <reflect/function.h>
#include <reflect/scope.h>
#include <reflect/context.h>

#include <adt/hash_map.h>
#include <adt/hash_map_str.h>

#include <stdlib.h>
#include <stdio.h>

#include <ruby.h>
#include <ruby/intern.h>

typedef struct loader_impl_rb_type
{
	VALUE inspect_module;
	VALUE inspect_module_data;

} * loader_impl_rb;

typedef struct loader_impl_rb_handle_type
{
	VALUE module;
	VALUE instance;

} * loader_impl_rb_handle;

int function_rb_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

void function_rb_interface_invoke(function func, function_impl impl, function_args args)
{
	loader_impl_rb_handle rb_handle = (loader_impl_rb_handle)impl;

	signature s = function_signature(func);

	const size_t args_size = signature_count(s);

	const char * func_name = function_name(func);

	VALUE result_value;

	if (args_size > 0)
	{
		VALUE args_value[args_size];

		size_t args_count;

		for (args_count = 0; args_count < args_size; ++args_count)
		{
			type t = signature_get_type(s, args_count);

			type_id id = type_index(t);

			printf("Type %p, %d\n", (void *)t, id);

			if (id == TYPE_INT)
			{
				int * value_ptr = (int *)(args[args_count]);

				args_value[args_count] = INT2NUM(*value_ptr);
			}
			else if (id == TYPE_DOUBLE)
			{
				double * value_ptr = (double *)(args[args_count]);

				args_value[args_count] = DBL2NUM(*value_ptr);
			}
			else
			{
				args_value[args_count] = Qnil;
			}
		}

		result_value = rb_funcallv(rb_handle->instance, rb_intern(func_name), args_size, args_value);

	}
	else
	{
		result_value = rb_funcall(rb_handle->instance, rb_intern(function_name(func)), 0);
	}

	printf("Function call result value:\n");

	rb_funcall(rb_mKernel, rb_intern("puts"), 1, result_value);
}

void function_rb_interface_destroy(function func, function_impl impl)
{
	(void)func;
	(void)impl;
}

function_interface function_rb_singleton()
{
	static struct function_interface_type rb_interface =
	{
		&function_rb_interface_create,
		&function_rb_interface_invoke,
		&function_rb_interface_destroy
	};

	return &rb_interface;
}

const char * rb_inspect_module_data()
{
	static const char inspect_script_str[] =

	/* TODO: re-implement this in C instead of evaluating Ruby code */

	"def self.class_method_params(klass, meth)\n"
		"captured_binding = nil\n"

		"TracePoint.new(:call) do |tp|\n"
			"captured_binding = tp.binding\n"
		"end.enable {\n"
			"obj = Class.new(klass) do\n"
				"def initialize\n"
				"end\n"
			"end.new\n"

			"meth_obj = klass.method(meth) rescue nil\n"

			"meth_obj = obj.method(meth) rescue nil if not meth_obj\n"

			"if meth_obj\n"
				"params = meth_obj.parameters\n"

				"opt_params = params.collect { |i| i.last if i.first == :opt }.compact\n"

				"required_params = [\"\"] * (params.size - opt_params.size)\n"

				"meth_obj.call(*required_params) rescue nil\n"

				"opt_params.each_with_object({}) do |i, hash|\n"
					"hash[i] = captured_binding.local_variable_get(i)\n"
				"end\n"
			"end\n"
		"}\n"
	"end\n"

	"def self.module_method_params(mod, meth)\n"
		"captured_binding = nil\n"

		"TracePoint.new(:call) do |tp|\n"
			"captured_binding = tp.binding\n"
		"end.enable {\n"
			"obj = Class.new(Object) do\n"
				"include mod\n"

				"def initialize\n"
				"end\n"
			"end.new\n"

			"meth_obj = mod.method(meth) rescue nil\n"

			"meth_obj = obj.method(meth) rescue nil if not meth_obj\n"

			"if meth_obj\n"
				"params = meth_obj.parameters\n"

				"opt_params = params.collect { |i| i.last if i.first == :opt }.compact\n"

				"required_params = [\"\"] * (params.size - opt_params.size)\n"

				"meth_obj.call(*required_params) rescue nil\n"

				"opt_params.each_with_object({}) do |i, hash|\n"
					"hash[i] = captured_binding.local_variable_get(i)\n"
				"end\n"
			"end\n"
		"}\n"
	"end\n";

	return inspect_script_str;
}

int rb_loader_impl_initialize_inspect(loader_impl_rb rb_impl)
{
	rb_impl->inspect_module = rb_define_module("Inspect");

	if (rb_impl->inspect_module != Qnil)
	{
		rb_impl->inspect_module_data = rb_str_new_cstr(rb_inspect_module_data());

		if (rb_impl->inspect_module_data != Qnil)
		{
			VALUE result = rb_funcall(rb_impl->inspect_module, rb_intern("module_eval"), 1, rb_impl->inspect_module_data);

			if (result != Qnil)
			{
				return 0;
			}
		}
	}

	return 1;
}

int rb_loader_impl_initialize_types(loader_impl impl)
{
	/* TODO: move this to loader_impl by passing the structure and loader_impl_derived callback */

	static struct
	{
		type_id id;
		const char * name;
	}
	type_id_name_pair[] =
	{
		{ TYPE_INT, "Fixnum" },
		{ TYPE_DOUBLE, "Float" }
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

loader_impl_data rb_loader_impl_initialize(loader_impl impl)
{
	loader_impl_rb rb_impl = malloc(sizeof(struct loader_impl_rb_type));

	(void)impl;

	if (rb_impl != NULL)
	{
		ruby_init();

		ruby_init_loadpath();

		if (rb_loader_impl_initialize_types(impl) == 0)
		{
			if (rb_loader_impl_initialize_inspect(rb_impl) == 0)
			{
				if (rb_gv_set("$VERBOSE", Qtrue) == Qtrue)
				{

					printf("Ruby loader initialized correctly\n");

					return rb_impl;
				}
			}
		}

		ruby_cleanup(0);

		free(rb_impl);
	}

	return NULL;
}

int rb_loader_impl_execution_path(loader_impl impl, loader_naming_path path)
{
	VALUE load_path_array = rb_gv_get("$:");

	VALUE path_value = rb_str_new_cstr(path);

	(void)impl;

	rb_ary_push(load_path_array, path_value);

	return 0;
}

VALUE rb_loader_impl_load_data(loader_impl impl, loader_naming_path path)
{
	VALUE load_path_array = rb_gv_get("$:");

	VALUE load_path_array_size = rb_funcall(load_path_array, rb_intern("size"), 0);

	VALUE module_path = rb_str_new_cstr(path);

	VALUE separator = rb_str_new_cstr("/");

	int index, size = FIX2INT(load_path_array_size);

	(void)impl;

	for (index = 0; index < size; ++index)
	{
		VALUE load_path_entry = rb_ary_entry(load_path_array, index);

		VALUE load_path = rb_str_append(load_path_entry, separator);

		VALUE module_absolute_path = rb_str_append(load_path, module_path);

		VALUE file_exists = rb_funcall(rb_cFile, rb_intern("exist?"), 1, module_absolute_path);

		if (file_exists == Qtrue)
		{
			VALUE module_data = rb_funcall(rb_cIO, rb_intern("read"), 1, module_absolute_path);

			if (module_data != Qnil)
			{
				return module_data;
			}
		}
	}

	return Qnil;
}

loader_handle rb_loader_impl_load(loader_impl impl, loader_naming_path path, loader_naming_name name)
{
	VALUE name_value = rb_str_new_cstr(name);

	VALUE name_capitalized = rb_funcall(name_value, rb_intern("capitalize"), 0);

	VALUE module = rb_define_module(RSTRING_PTR(name_capitalized));

	if (module != Qnil)
	{
		VALUE module_data = rb_loader_impl_load_data(impl, path);

		if (module_data != Qnil)
		{
			VALUE result = rb_funcall(module, rb_intern("module_eval"), 1, module_data);

			if (result != Qnil)
			{
				loader_impl_rb_handle handle = malloc(sizeof(struct loader_impl_rb_handle_type));

				if (handle != NULL)
				{
					handle->module = module;

					handle->instance = rb_funcall(rb_cClass, rb_intern("new"), 1, rb_cObject);

					rb_extend_object(handle->instance, handle->module);

					printf("Ruby module %s loaded\n", path);

					return (loader_handle)handle;
				}
			}
			else
			{
				VALUE exception = rb_errinfo();

				printf("Ruby loader error (%s)\n", RSTRING_PTR(exception));
			}
		}
	}

	return NULL;
}

int rb_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_rb_handle rb_handle = (loader_impl_rb_handle)handle;

	(void)impl;

	if (rb_handle != NULL)
	{
		free(rb_handle);

		return 0;
	}

	return 1;
}

int rb_loader_impl_discover_func(loader_impl impl, loader_impl_rb rb_impl, VALUE parameter_array, function f)
{
	if (rb_impl != NULL && parameter_array != Qnil)
	{
		signature s = function_signature(f);

		if (s != NULL)
		{
			size_t index, size = signature_count(s);

			for (index = 0; index < size; ++index)
			{
				VALUE parameter_pair = rb_ary_entry(parameter_array, index);

				VALUE parameter_symbol = rb_ary_entry(parameter_pair, 0);

				VALUE parameter_name = rb_funcall(parameter_symbol, rb_intern("id2name"), 0);

				const char * parameter_name_str = RSTRING_PTR(parameter_name);

				VALUE parameter_value = rb_ary_entry(parameter_pair, 1);

				VALUE parameter_value_type = rb_funcall(parameter_value, rb_intern("class"), 0);

				VALUE parameter_value_type_name = rb_funcall(parameter_value_type, rb_intern("to_s"), 0);

				const char * parameter_value_type_name_str = RSTRING_PTR(parameter_value_type_name);

				printf("Parameter (%ld) <%s> Type %s %ld\n", index,
					parameter_name_str, parameter_value_type_name_str, parameter_value_type);

				signature_set(s, index, parameter_name_str, loader_impl_type(impl, parameter_value_type_name_str));
			}

			return 0;
		}
	}

	return 1;
}

int rb_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_rb_handle rb_handle = (loader_impl_rb_handle)handle;

	VALUE instance_methods = rb_funcall(rb_handle->module, rb_intern("instance_methods"), 0);

	VALUE methods_size = rb_funcall(instance_methods, rb_intern("size"), 0);

	int index, size = FIX2INT(methods_size);

	printf("Ruby loader discovering:\n");

	for (index = 0; index < size; ++index)
	{
		VALUE method = rb_ary_entry(instance_methods, index);

		if (method != Qnil)
		{
			loader_impl_rb rb_impl = loader_impl_get(impl);

			VALUE method_name = rb_funcall(method, rb_intern("id2name"), 0);

			const char * method_name_str = RSTRING_PTR(method_name);

			VALUE parameter_map = rb_funcall(rb_impl->inspect_module, rb_intern("class_method_params"), 2, rb_handle->instance, method);

			VALUE parameter_array = rb_funcall(parameter_map, rb_intern("to_a"), 0);

			VALUE parameters_size = rb_funcall(parameter_array, rb_intern("size"), 0);

			int parameters_size_integer = FIX2INT(parameters_size);

			function f = function_create(method_name_str, parameters_size_integer, rb_handle, &function_rb_singleton);

			if (f != NULL && rb_loader_impl_discover_func(impl, rb_impl, parameter_array, f) == 0)
			{
				scope sp = context_scope(ctx);

				scope_define(sp, method_name_str, f);

				printf("Function %s <%p> (%d)\n", method_name_str, (void *)f, parameters_size_integer);
			}
		}
	}

	return 0;
}

int rb_loader_impl_destroy(loader_impl impl)
{
	loader_impl_rb rb_impl = loader_impl_get(impl);

	if (rb_impl != NULL)
	{
		ruby_cleanup(0);

		free(rb_impl);

		return 0;
	}

	return 1;
}
