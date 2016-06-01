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

typedef struct loader_impl_rb_type
{
	void * todo;

} * loader_impl_rb;

int function_rb_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

void function_rb_interface_invoke(function func, function_impl impl, function_args args)
{
	(void)func;
	(void)impl;
	(void)args;
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

loader_impl_data rb_loader_impl_initialize(loader_impl impl)
{
	loader_impl_rb rb_impl = malloc(sizeof(struct loader_impl_rb_type));

	(void)impl;

	if (rb_impl != NULL)
	{
		ruby_init();

		ruby_init_loadpath();

		printf("Ruby loader initialized correctly\n");

		return rb_impl;
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

		VALUE file_exists = rb_funcall(rb_cFile, rb_intern("exists?"), 1, module_absolute_path);

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
	VALUE * module = malloc(sizeof(VALUE));

	(void)impl;

	if (module != NULL)
	{
		*module = rb_define_module(name);

		if (*module != Qnil)
		{
			VALUE module_data = rb_loader_impl_load_data(impl, path);

			if (module_data != Qnil)
			{
				VALUE result = rb_funcall(*module, rb_intern("module_eval"), 1, module_data);

				if (result != Qnil)
				{
					printf("Ruby module %s loaded\n", path);

					return (loader_handle)module;
				}
				else
				{
					VALUE exception = rb_errinfo();

					printf("Ruby loader error (%s)\n", RSTRING_PTR(exception));
				}
			}
		}

		free(module);
	}

	return NULL;
}

int rb_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	VALUE * module = (VALUE *)handle;

	(void)impl;

	if (module != NULL)
	{
		free(module);

		return 0;
	}

	return 1;
}

int rb_loader_impl_discover_func(loader_impl impl, VALUE method, VALUE parameters, function f)
{
	signature s = function_signature(f);

	(void)impl;
	(void)method;

	if (s != NULL)
	{
		size_t index, size = signature_count(s);

		for (index = 0; index < size; ++index)
		{
			VALUE parameter = rb_ary_entry(parameters, index);

			/* VALUE parameter_mode = rb_ary_entry(parameter, 0); */

			VALUE parameter_symbol = rb_ary_entry(parameter, 1);

			VALUE parameter_name = rb_funcall(parameter_symbol, rb_intern("id2name"), 0);

			char const * parameter_name_str = RSTRING_PTR(parameter_name);

			printf("Parameter %s (%ld):\n", parameter_name_str, index);

			rb_funcall(rb_mKernel, rb_intern("puts"), 1, parameter);

			signature_set(s, index, parameter_name_str, NULL);
		}

		return 0;
	}

	return 1;
}

int rb_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	VALUE * module = (VALUE *)handle;

	VALUE instance_methods = rb_funcall(*module, rb_intern("instance_methods"), 0);

	VALUE instance_methods_size = rb_funcall(instance_methods, rb_intern("size"), 0);

	int index, size = FIX2INT(instance_methods_size);

	(void)impl;
	(void)ctx;

	printf("Ruby loader discovering:\n");

	for (index = 0; index < size; ++index)
	{
		VALUE * method = malloc(sizeof(VALUE));

		if (method != NULL)
		{
			*method = rb_ary_entry(instance_methods, index);

			if (*method != Qnil)
			{
				VALUE method_name = rb_funcall(*method, rb_intern("id2name"), 0);

				const char * method_name_str = RSTRING_PTR(method_name);

				VALUE method_ptr = rb_funcall(*module, rb_intern("instance_method"), 1, *method);

				VALUE parameters = rb_funcall(method_ptr, rb_intern("parameters"), 0);

				VALUE parameters_size = rb_funcall(parameters, rb_intern("size"), 0);

				int parameters_size_integer = FIX2INT(parameters_size);

				function f = function_create(method_name_str, parameters_size_integer, method, &function_rb_singleton);

				if (f != NULL && rb_loader_impl_discover_func(impl, *method, parameters, f) == 0)
				{
					scope sp = context_scope(ctx);

					scope_define(sp, method_name_str, f);

					printf("Function %s <%p> (%d)\n", method_name_str, (void *)f, parameters_size_integer);

					rb_funcall(rb_mKernel, rb_intern("puts"), 3, *method, method_ptr, parameters);
				}
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
