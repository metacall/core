/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading ruby code at run-time into a process.
 *
 */

#include <rb_loader/rb_loader_impl.h>
#include <rb_loader/rb_loader_impl_key.h>

#include <loader/loader_impl.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

#include <adt/adt_set.h>
#include <adt/adt_vector.h>

#include <log/log.h>

#include <stdlib.h>

#if (defined(_WIN32) || defined(_WIN64)) && defined(boolean)
#	undef boolean
#endif

#include <ruby.h>

#define LOADER_IMPL_RB_FUNCTION_ARGS_SIZE 0x10

typedef struct loader_impl_rb_module_type
{
	VALUE module;
	VALUE instance;
	set function_map;

} * loader_impl_rb_module;

typedef struct loader_impl_rb_handle_type
{
	vector modules;

} * loader_impl_rb_handle;

typedef struct loader_impl_rb_function_type
{
	loader_impl_rb_module rb_module;
	ID method_id;
	VALUE args_hash;

} * loader_impl_rb_function;

int function_rb_interface_create(function func, function_impl impl)
{
	signature s = function_signature(func);

	/* Set to null, deduced dynamically */
	signature_set_return(s, NULL);

	(void)impl;

	return 0;
}

function_return function_rb_interface_invoke(function func, function_impl impl, function_args args)
{
	loader_impl_rb_function rb_function = (loader_impl_rb_function)impl;

	signature s = function_signature(func);

	const size_t args_size = signature_count(s);

	VALUE result_value = Qnil;

	if (args_size > LOADER_IMPL_RB_FUNCTION_ARGS_SIZE)
	{
		return NULL;
	}

	if (args_size > 0)
	{
		size_t args_count;

		VALUE args_value[LOADER_IMPL_RB_FUNCTION_ARGS_SIZE];

		for (args_count = 0; args_count < args_size; ++args_count)
		{
			type t = signature_get_type(s, args_count);

			type_id id = type_index(t);

			log_write("metacall", LOG_LEVEL_DEBUG, "Type %p, %d", (void *)t, id);

			if (id == TYPE_BOOL)
			{
				boolean * value_ptr = (boolean *)(args[args_count]);

				args_value[args_count] = (*value_ptr == 0L) ? Qfalse : Qtrue;
			}
			else if (id == TYPE_INT)
			{
				int * value_ptr = (int *)(args[args_count]);

				args_value[args_count] = INT2NUM(*value_ptr);
			}
			else if (id == TYPE_LONG)
			{
				long * value_ptr = (long *)(args[args_count]);

				args_value[args_count] = LONG2NUM(*value_ptr);
			}
			else if (id == TYPE_FLOAT)
			{
				float * value_ptr = (float *)(args[args_count]);

				args_value[args_count] = DBL2NUM((double)*value_ptr);
			}
			else if (id == TYPE_DOUBLE)
			{
				double * value_ptr = (double *)(args[args_count]);

				args_value[args_count] = DBL2NUM(*value_ptr);
			}
			else if (id == TYPE_STRING)
			{
				const char * value_ptr = (const char *)(args[args_count]);

				args_value[args_count] = rb_str_new_cstr(value_ptr);
			}
			else
			{
				args_value[args_count] = Qnil;
			}

			rb_hash_aset(rb_function->args_hash, ID2SYM(rb_intern(signature_get_name(s, args_count))), args_value[args_count]);
		}

		result_value = rb_funcall(rb_function->rb_module->instance, rb_intern("send"), 2,
			ID2SYM(rb_function->method_id), rb_function->args_hash);
	}
	else
	{
		result_value = rb_funcall(rb_function->rb_module->instance, rb_function->method_id, 0);
	}

	if (result_value != Qnil)
	{
		int result_type = TYPE(result_value);

		value v = NULL;

		if (result_type == T_TRUE)
		{
			boolean b = 1L;

			v = value_create_bool(b);
		}
		else if (result_type == T_FALSE)
		{
			boolean b = 0L;

			v = value_create_bool(b);
		}
		else if (result_type == T_FIXNUM)
		{
			int i = FIX2INT(result_value);

			v = value_create_int(i);
		}
		else if (result_type == T_BIGNUM)
		{
			long l = NUM2LONG(result_value);

			v = value_create_long(l);
		}
		else if (result_type == T_FLOAT)
		{
			double d = NUM2DBL(result_value);

			v = value_create_double(d);
		}
		else if (result_type == T_STRING)
		{
			long length = RSTRING_LEN(result_value);

			char * str = StringValuePtr(result_value);

			if (length > 0 && str != NULL)
			{
				v = value_create_string(str, (size_t)length);
			}
		}

		return v;
	}

	return NULL;
}

void function_rb_interface_destroy(function func, function_impl impl)
{
	loader_impl_rb_function rb_function = (loader_impl_rb_function)impl;

	(void)func;

	if (rb_function != NULL)
	{
		free(rb_function);
	}
}

function_interface function_rb_singleton(void)
{
	static struct function_interface_type rb_interface =
	{
		&function_rb_interface_create,
		&function_rb_interface_invoke,
		&function_rb_interface_destroy
	};

	return &rb_interface;
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
		/*
		{ TYPE_BOOL, "Boolean" },
		*/
		{ TYPE_INT, "Fixnum" },
		{ TYPE_LONG, "Bignum" },
		{ TYPE_DOUBLE, "Float" },
		{ TYPE_STRING, "String" }
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
	static struct rb_loader_impl_type
	{
		void * unused;

	}
	rb_loader_impl_unused =
	{
		NULL
	};

	(void)impl;

	ruby_init();

	ruby_init_loadpath();

	if (rb_loader_impl_initialize_types(impl) != 0)
	{
		ruby_cleanup(0);

		return NULL;
	}

	if (rb_gv_set("$VERBOSE", Qtrue) != Qtrue)
	{
		ruby_cleanup(0);

		return NULL;
	}

	log_write("metacall", LOG_LEVEL_DEBUG, "Ruby loader initialized correctly");

	return (loader_impl_data)&rb_loader_impl_unused;
}

int rb_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	VALUE load_path_array = rb_gv_get("$:");

	VALUE path_value = rb_str_new_cstr(path);

	(void)impl;

	rb_ary_push(load_path_array, path_value);

	return 0;
}

VALUE rb_loader_impl_load_data_absolute(VALUE module_absolute_path)
{
	VALUE file_exists = rb_funcall(rb_cFile, rb_intern("exist?"), 1, module_absolute_path);

	log_write("metacall", LOG_LEVEL_DEBUG, "RBPATH: %s", RSTRING_PTR(module_absolute_path));

	if (file_exists == Qtrue)
	{
		VALUE module_data = rb_funcall(rb_cIO, rb_intern("read"), 1, module_absolute_path);

		if (module_data != Qnil)
		{
			return module_data;
		}
	}

	return Qnil;
}

VALUE rb_loader_impl_load_data(loader_impl impl, const loader_naming_path path)
{
	VALUE load_path_array = rb_gv_get("$:");

	VALUE load_path_array_size = rb_funcall(load_path_array, rb_intern("size"), 0);

	VALUE module_path = rb_str_new_cstr(path);

	VALUE separator = rb_str_new_cstr("/");

	int index, size = FIX2INT(load_path_array_size);

	VALUE module = rb_loader_impl_load_data_absolute(module_path);

	(void)impl;

	if (module != Qnil)
	{
		return module;
	}

	for (index = 0; index < size; ++index)
	{
		VALUE load_path_entry = rb_ary_entry(load_path_array, index);

		VALUE load_path = rb_str_append(load_path_entry, separator);

		VALUE module_absolute_path = rb_str_append(load_path, module_path);

		module = rb_loader_impl_load_data_absolute(module_absolute_path);

		if (module != Qnil)
		{
			return module;
		}
	}

	return Qnil;
}

loader_impl_rb_module rb_loader_impl_load_from_file_module(loader_impl impl, const loader_naming_path path, const loader_naming_name name)
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
				loader_impl_rb_module rb_module = malloc(sizeof(struct loader_impl_rb_module_type));

				if (rb_module != NULL)
				{
					rb_module->module = module;

					rb_module->instance = rb_funcall(rb_cClass, rb_intern("new"), 1, rb_cObject);

					rb_extend_object(rb_module->instance, rb_module->module);

					rb_include_module(rb_module->instance, rb_module->module);

					rb_module->function_map = set_create(&hash_callback_str, &comparable_callback_str);

					if (!(rb_module->function_map != NULL && rb_loader_impl_key_parse(RSTRING_PTR(module_data), rb_module->function_map) == 0))
					{
						set_destroy(rb_module->function_map);

						free(rb_module);

						return NULL;
					}

					log_write("metacall", LOG_LEVEL_DEBUG, "Ruby module %s loaded", path);

					rb_loader_impl_key_print(rb_module->function_map);

					return rb_module;
				}
			}
			else
			{
				VALUE exception = rb_errinfo();

				log_write("metacall", LOG_LEVEL_DEBUG, "Ruby loader error (%s)", RSTRING_PTR(exception));
			}
		}
	}

	return NULL;
}


loader_handle rb_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	loader_impl_rb_handle handle = malloc(sizeof(struct loader_impl_rb_handle_type));

	size_t iterator;

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid ruby handle allocation");

		return NULL;
	}

	handle->modules = vector_create_reserve(sizeof(loader_impl_rb_module), size);

	if (handle->modules == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid ruby modules vector allocation");

		free(handle);

		return NULL;
	}

	for (iterator = 0; iterator < size; ++iterator)
	{
		loader_impl_rb_module rb_module;

		loader_naming_name module_name;

		loader_path_get_name(paths[iterator], module_name);

		rb_module = rb_loader_impl_load_from_file_module(impl, paths[iterator], module_name);

		if (rb_module == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid ruby module loading %s", paths[iterator]);

			continue;
		}

		vector_push_back(handle->modules, &rb_module);
	}

	return (loader_handle)handle;
}

loader_impl_rb_module rb_loader_impl_load_from_memory_module(loader_impl impl, const loader_naming_name name, const loader_naming_extension extension, const char * buffer, size_t size)
{
	VALUE name_value = rb_str_new_cstr(name);

	VALUE name_capitalized = rb_funcall(name_value, rb_intern("capitalize"), 0);

	VALUE module = rb_define_module(RSTRING_PTR(name_capitalized));

	(void)impl;
	(void)size;

	if (module != Qnil)
	{
		VALUE module_data = rb_str_new_cstr(buffer);

		if (module_data != Qnil)
		{
			VALUE result = rb_funcall(module, rb_intern("module_eval"), 1, module_data);

			if (result != Qnil)
			{
				loader_impl_rb_module rb_module = malloc(sizeof(struct loader_impl_rb_module_type));

				if (rb_module != NULL)
				{
					rb_module->module = module;

					rb_module->instance = rb_funcall(rb_cClass, rb_intern("new"), 1, rb_cObject);

					rb_extend_object(rb_module->instance, rb_module->module);

					rb_include_module(rb_module->instance, rb_module->module);

					rb_module->function_map = set_create(&hash_callback_str, &comparable_callback_str);

					if (!(rb_module->function_map != NULL && rb_loader_impl_key_parse(RSTRING_PTR(module_data), rb_module->function_map) == 0))
					{
						set_destroy(rb_module->function_map);

						free(rb_module);

						return NULL;
					}

					log_write("metacall", LOG_LEVEL_DEBUG, "Ruby module %s.%s loaded", name, extension);

					rb_loader_impl_key_print(rb_module->function_map);

					return rb_module;
				}
			}
			else
			{
				VALUE exception = rb_errinfo();

				log_write("metacall", LOG_LEVEL_DEBUG, "Ruby loader error (%s)", RSTRING_PTR(exception));
			}
		}
	}

	return NULL;
}

loader_handle rb_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const loader_naming_extension extension, const char * buffer, size_t size)
{
	loader_impl_rb_handle handle = malloc(sizeof(struct loader_impl_rb_handle_type));

	loader_impl_rb_module rb_module;

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid ruby handle allocation");

		return NULL;
	}

	handle->modules = vector_create_reserve(sizeof(loader_impl_rb_module), 1);

	if (handle->modules == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid ruby modules vector allocation");

		free(handle);

		return NULL;
	}

	rb_module = rb_loader_impl_load_from_memory_module(impl, name, extension, buffer, size);

	if (rb_module == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid ruby module loading from memory");

		vector_destroy(handle->modules);

		free(handle);

		return NULL;
	}

	vector_push_back(handle->modules, &rb_module);

	return (loader_handle)handle;
}

loader_handle rb_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	/* TODO */

	(void)impl;
	(void)path;

	return NULL;
}

int rb_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_rb_handle rb_handle = (loader_impl_rb_handle)handle;

	(void)impl;

	if (rb_handle != NULL)
	{
		size_t iterator, size = vector_size(rb_handle->modules);

		for (iterator = 0; iterator < size; ++iterator)
		{
			loader_impl_rb_module * rb_module = vector_at(rb_handle->modules, iterator);

			set_destroy((*rb_module)->function_map);
		}

		vector_destroy(rb_handle->modules);

		free(rb_handle);

		return 0;
	}

	return 1;
}

int rb_loader_impl_discover_func(loader_impl impl, function f, rb_function_parser function_parser)
{
	signature s = function_signature(f);

	if (s != NULL)
	{
		const size_t size = signature_count(s);

		size_t index;

		for (index = 0; index < size; ++index)
		{
			signature_set(s, index, function_parser->params[index].name, loader_impl_type(impl, function_parser->params[index].type));
		}

		return 0;
	}

	return 1;
}

loader_impl_rb_function rb_function_create(loader_impl_rb_module rb_module, ID id)
{
	loader_impl_rb_function rb_function = malloc(sizeof(struct loader_impl_rb_function_type));

	if (rb_function != NULL)
	{
		rb_function->rb_module = rb_module;
		rb_function->method_id = id;
		rb_function->args_hash = rb_hash_new();

		return rb_function;
	}

	return NULL;
}

int rb_loader_impl_discover_module(loader_impl impl, loader_impl_rb_module rb_module, context ctx)
{
	VALUE instance_methods = rb_funcall(rb_module->module, rb_intern("instance_methods"), 0);

	VALUE methods_size = rb_funcall(instance_methods, rb_intern("size"), 0);

	int index, size = FIX2INT(methods_size);

	log_write("metacall", LOG_LEVEL_DEBUG, "Ruby loader discovering:");

	for (index = 0; index < size; ++index)
	{
		VALUE method = rb_ary_entry(instance_methods, index);

		if (method != Qnil)
		{
			VALUE method_name = rb_funcall(method, rb_intern("id2name"), 0);

			const char * method_name_str = RSTRING_PTR(method_name);

			rb_function_parser function_parser = set_get(rb_module->function_map, (set_key)method_name_str);

			loader_impl_rb_function rb_function = NULL;

			if (function_parser == NULL)
			{
				continue;
			}

			rb_function = rb_function_create(rb_module, rb_intern(method_name_str));

			if (rb_function)
			{
				function f = function_create(method_name_str, function_parser->params_size, rb_function, &function_rb_singleton);

				if (f != NULL && rb_loader_impl_discover_func(impl, f, function_parser) == 0)
				{
					scope sp = context_scope(ctx);

					scope_define(sp, function_name(f), f);

					log_write("metacall", LOG_LEVEL_DEBUG, "Function %s <%p> (%d)", method_name_str, (void *)f, function_parser->params_size);
				}
				else
				{
					return 1;
				}
			}
			else
			{
				return 1;
			}
		}
	}

	return 0;
}

int rb_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_rb_handle rb_handle = (loader_impl_rb_handle)handle;

	size_t iterator, size = vector_size(rb_handle->modules);

	int result = 0;

	for (iterator = 0; iterator < size; ++iterator)
	{
		loader_impl_rb_module * rb_module = vector_at(rb_handle->modules, iterator);

		if (rb_loader_impl_discover_module(impl, *rb_module, ctx) != 0)
		{
			result = 1;
		}
	}

	return result;
}

int rb_loader_impl_destroy(loader_impl impl)
{
	(void)impl;

	return ruby_cleanup(0);
}
