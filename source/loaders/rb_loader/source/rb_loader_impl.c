/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading ruby code at run-time into a process.
 *
 */

#include <rb_loader/rb_loader_impl.h>
#include <rb_loader/rb_loader_impl_parser.h>

#include <loader/loader_impl.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_type_id.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

#include <adt/adt_set.h>
#include <adt/adt_vector.h>

#include <log/log.h>

#include <stdlib.h>
#include <stdio.h>

#if (defined(_WIN32) || defined(_WIN64)) && defined(boolean)
#	undef boolean
#endif

#include <ruby.h>

#define LOADER_IMPL_RB_FUNCTION_ARGS_SIZE 0x10
#define LOADER_IMPL_RB_PROTECT_ARGS_SIZE 0x10

typedef struct loader_impl_rb_module_type
{
	ID id;
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
	loader_impl impl;

} * loader_impl_rb_function;

int function_rb_interface_create(function func, function_impl impl)
{
	signature s = function_signature(func);

	/* Set to null, deduced dynamically */
	signature_set_return(s, NULL);

	(void)impl;

	return 0;
}

const char * rb_type_deserialize(VALUE v, value * result)
{
	if (v != Qnil)
	{
		int v_type = TYPE(v);

		if (v_type == T_TRUE)
		{
			boolean b = 1L;

			*result = value_create_bool(b);

			return "Boolean";
		}
		else if (v_type == T_FALSE)
		{
			boolean b = 0L;

			*result = value_create_bool(b);

			return "Boolean";
		}
		else if (v_type == T_FIXNUM)
		{
			int i = FIX2INT(v);

			*result = value_create_int(i);

			return "Fixnum";
		}
		else if (v_type == T_BIGNUM)
		{
			long l = NUM2LONG(v);

			*result = value_create_long(l);

			return "Bignum";
		}
		else if (v_type == T_FLOAT)
		{
			double d = NUM2DBL(v);

			*result = value_create_double(d);

			return "Float";
		}
		else if (v_type == T_STRING)
		{
			long length = RSTRING_LEN(v);

			char * str = StringValuePtr(v);

			if (length > 0 && str != NULL)
			{
				*result = value_create_string(str, (size_t)length);
			}

			return "String";
		}
		else if (v_type == T_ARRAY)
		{
			size_t iterator, size = RARRAY_LEN(v);

			VALUE * array_ptr = RARRAY_PTR(v);

			*result = value_create_array(NULL, size);

			if (size > 0 && *result != NULL)
			{
				value * v_array_ptr = value_to_array(*result);

				for (iterator = 0; iterator < size; ++iterator, ++array_ptr)
				{
					(void)rb_type_deserialize(*array_ptr, &v_array_ptr[iterator]);
				}
			}

			return "Array";
		}
		else if (v_type == T_OBJECT)
		{
			// TODO

			return "Object";
		}
	}

	return NULL;
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
		enum function_rb_interface_invoke_id
		{
			FUNCTION_RB_UNKNOWN,
			FUNCTION_RB_TYPED,
			FUNCTION_RB_DUCKTYPED,
			FUNCTION_RB_MIXED
		} invoke_type = FUNCTION_RB_UNKNOWN;

		size_t args_count, ducktype_args_count = 0;

		VALUE args_value[LOADER_IMPL_RB_FUNCTION_ARGS_SIZE + 1];

		for (args_count = 0; args_count < args_size; ++args_count)
		{
			type t = signature_get_type(s, args_count);

			type_id id = TYPE_INVALID;

			if (t == NULL)
			{
				id = value_type_id((value)args[args_count]);

				if (invoke_type == FUNCTION_RB_UNKNOWN)
				{
					invoke_type = FUNCTION_RB_DUCKTYPED;
				}
			}
			else
			{
				if (invoke_type == FUNCTION_RB_UNKNOWN)
				{
					invoke_type = FUNCTION_RB_TYPED;
				}
				else if (invoke_type == FUNCTION_RB_DUCKTYPED)
				{
					invoke_type = FUNCTION_RB_MIXED;

					ducktype_args_count = args_count;
				}

				id = type_index(t);
			}

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

			if (t != NULL)
			{
				rb_hash_aset(rb_function->args_hash, ID2SYM(rb_intern(signature_get_name(s, args_count))), args_value[args_count]);
			}
		}

		if (invoke_type == FUNCTION_RB_TYPED)
		{
			result_value = rb_funcall(rb_function->rb_module->instance, rb_intern("send"), 2,
				ID2SYM(rb_function->method_id), rb_function->args_hash);
		}
		else if (invoke_type == FUNCTION_RB_DUCKTYPED)
		{
			/* TODO: Improve this horrible code in the future */
			for (args_count = args_size; args_count > 0; --args_count)
			{
				args_value[args_count] = args_value[args_count - 1];
			}

			args_value[0] = ID2SYM(rb_function->method_id);

			result_value = rb_funcall2(rb_function->rb_module->instance, rb_intern("send"), 1 + args_size, args_value);
		}
		else if (invoke_type == FUNCTION_RB_MIXED)
		{
			/* TODO: Improve this horrible code in the future */
			for (args_count = ducktype_args_count; args_count > 0; --args_count)
			{
				args_value[args_count] = args_value[args_count - 1];
			}

			args_value[0] = ID2SYM(rb_function->method_id);

			args_value[ducktype_args_count + 1] = rb_function->args_hash;

			result_value = rb_funcall2(rb_function->rb_module->instance, rb_intern("send"), 1 + ducktype_args_count + 1, args_value);
		}
	}
	else
	{
		result_value = rb_funcall(rb_function->rb_module->instance, rb_function->method_id, 0);
	}

	if (result_value != Qnil)
	{
		value v = NULL;

		const char * v_type_name = rb_type_deserialize(result_value, &v);

		signature_set_return(s, loader_impl_type(rb_function->impl, v_type_name));

		return v;
	}

	return NULL;
}

function_return function_rb_interface_await(function func, function_impl impl, function_args args, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void * context)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)resolve_callback;
	(void)reject_callback;
	(void)context;

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
		&function_rb_interface_await,
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
		{ TYPE_BOOL, "Boolean" },
		{ TYPE_INT, "Fixnum" },
		{ TYPE_LONG, "Bignum" },
		{ TYPE_DOUBLE, "Float" },
		{ TYPE_STRING, "String" },
		{ TYPE_ARRAY, "Array" }
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

loader_impl_data rb_loader_impl_initialize(loader_impl impl, configuration config, loader_host host)
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
	(void)config;

	log_copy(host->log);

	/* Initialize Ruby */
	{
		RUBY_INIT_STACK;

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
	}

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

VALUE rb_loader_impl_module_eval(VALUE module, VALUE module_data)
{
	const int argc = 1;
	VALUE result;
	VALUE args[argc];

	args[0] = module_data;

	result = rb_mod_module_eval(1, args, module);

	if (result == Qnil)
	{
		VALUE exception;

		log_write("metacall", LOG_LEVEL_ERROR, "Ruby module evaluation failed");

		exception = rb_gv_get("$!");

		if (RTEST(exception))
		{
			VALUE inspect, backtrace;

			inspect = rb_inspect(exception);

			rb_io_puts(1, &inspect, rb_stderr);

			backtrace = rb_funcall(exception, rb_intern("backtrace"), 0);

			rb_io_puts(1, &backtrace, rb_stderr);

			rb_raise(rb_eLoadError, "Invalid module evaluation");

			return Qnil;
		}
	}

	return result;
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
			VALUE result = rb_loader_impl_module_eval(module, module_data);

			if (result != Qnil)
			{
				loader_impl_rb_module rb_module = malloc(sizeof(struct loader_impl_rb_module_type));

				if (rb_module != NULL)
				{
					rb_module->id = rb_to_id(name_capitalized);

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

loader_impl_rb_module rb_loader_impl_load_from_memory_module(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size)
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
			VALUE result = rb_loader_impl_module_eval(module, module_data);

			if (result != Qnil)
			{
				loader_impl_rb_module rb_module = malloc(sizeof(struct loader_impl_rb_module_type));

				if (rb_module != NULL)
				{
					rb_module->id = rb_to_id(name_capitalized);

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

					log_write("metacall", LOG_LEVEL_DEBUG, "Ruby module %s loaded", name);

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

loader_handle rb_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size)
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

	rb_module = rb_loader_impl_load_from_memory_module(impl, name, buffer, size);

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

int rb_loader_impl_clear_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	VALUE * module = (VALUE *)args;

	rb_function_parser function_parser = (rb_function_parser)val;

	VALUE name = rb_str_new_cstr(function_parser->name);

	(void)s;
	(void)key;

	rb_undef(*module, rb_to_id(name));

	return 0;
}

int rb_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_rb_handle rb_handle = (loader_impl_rb_handle)handle;

	size_t iterator, size;

	(void)impl;

	if (rb_handle == NULL)
	{
		return 1;
	}

	size = vector_size(rb_handle->modules);

	for (iterator = 0; iterator < size; ++iterator)
	{
		loader_impl_rb_module * rb_module = vector_at(rb_handle->modules, iterator);

		/* Undef all methods */
		set_iterate((*rb_module)->function_map, &rb_loader_impl_clear_cb_iterate, (set_cb_iterate_args)&((*rb_module)->module));

		/* Remove module */
		if (rb_is_const_id((*rb_module)->id))
		{
			VALUE result = rb_const_remove(rb_cObject, (*rb_module)->id);

			/* TODO: Handle result */
			(void)result;
		}

		set_destroy((*rb_module)->function_map);
	}

	vector_destroy(rb_handle->modules);

	free(rb_handle);

	return 0;
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

					rb_function->impl = impl;

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
