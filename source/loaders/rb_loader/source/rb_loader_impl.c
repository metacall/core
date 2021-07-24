/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading ruby code at run-time into a process.
 *
 */

#include <rb_loader/rb_loader_impl.h>
#include <rb_loader/rb_loader_impl_parser.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>
#include <reflect/reflect_type_id.h>

#include <adt/adt_set.h>
#include <adt/adt_vector.h>

#include <log/log.h>

#include <stdio.h>
#include <stdlib.h>

#if (defined(_WIN32) || defined(_WIN64)) && defined(boolean)
	#undef boolean
#endif

/* Disable warnings from Ruby */
#if defined(__GNUC__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wredundant-decls"
	#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#include <ruby.h>

/* Disable warnings from Ruby */
#if defined(__GNUC__)
	#pragma GCC diagnostic pop
#endif

#define LOADER_IMPL_RB_FUNCTION_ARGS_SIZE 0x10
#define LOADER_IMPL_RB_PROTECT_ARGS_SIZE  0x10

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

typedef struct loader_impl_rb_class_type
{
	VALUE class;
	loader_impl impl;

} * loader_impl_rb_class;

typedef struct loader_impl_rb_object_type
{
	VALUE object;
	VALUE object_class;
	loader_impl impl;

} * loader_impl_rb_object;

typedef struct loader_impl_rb_module_eval_protect_type
{
	int argc;
	VALUE *argv;
	VALUE module;
} * loader_impl_rb_module_eval_protect;

class_interface rb_class_interface_singleton();
object_interface rb_object_interface_singleton();

int function_rb_interface_create(function func, function_impl impl)
{
	signature s = function_signature(func);

	/* Set to null, deduced dynamically */
	signature_set_return(s, NULL);

	(void)impl;

	return 0;
}

const char *rb_type_deserialize(VALUE v, value *result)
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

		char *str = StringValuePtr(v);

		if (length > 0 && str != NULL)
		{
			*result = value_create_string(str, (size_t)length);
		}

		return "String";
	}
	else if (v_type == T_ARRAY)
	{
		size_t iterator, size = RARRAY_LEN(v);

		VALUE *array_ptr = RARRAY_PTR(v);

		*result = value_create_array(NULL, size);

		if (size > 0 && *result != NULL)
		{
			value *v_array_ptr = value_to_array(*result);

			for (iterator = 0; iterator < size; ++iterator, ++array_ptr)
			{
				(void)rb_type_deserialize(*array_ptr, &v_array_ptr[iterator]);
			}
		}

		return "Array";
	}
	else if (v_type == T_NIL)
	{
		*result = value_create_null();

		return "NilClass";
	}
	else if (v_type == T_OBJECT)
	{
		loader_impl_rb_object rb_obj = malloc(sizeof(struct loader_impl_rb_object_type));

		VALUE object_class = rb_obj_class(v);

		value obj_cls_val = NULL;
		rb_type_deserialize(object_class, &obj_cls_val);

		klass kls = value_to_class(obj_cls_val);

		VALUE inspect = rb_inspect(v);
		char *inspect_str = StringValuePtr(inspect);

		/* inspect vs class_name(kls) as object name ?*/
		object o = object_create(inspect_str, rb_obj, &rb_object_interface_singleton, kls);

		// rb_obj->impl = impl;
		rb_obj->object = v;
		rb_obj->object_class = object_class;

		*result = value_create_object(o);

		return "Object";
	}
	else if (v_type == T_CLASS)
	{
		loader_impl_rb_class rb_cls = malloc(sizeof(struct loader_impl_rb_class_type));

		VALUE class_name = rb_funcall(v, rb_intern("name"), 0);
		const char *class_name_str = RSTRING_PTR(class_name);

		klass c = class_create(class_name_str, rb_cls, &rb_class_interface_singleton);

		// rb_cls->impl = impl;
		rb_cls->class = v;

		*result = value_create_class(c);

		return "Class";
	}

	return NULL;
}

VALUE rb_type_serialize(value v)
{
	if (v == NULL)
	{
		return Qnil;
	}

	type_id v_type = value_type_id(v);

	if (v_type == TYPE_BOOL)
	{
		return (value_to_bool(v) == 0L) ? Qfalse : Qtrue;
	}
	else if (v_type == TYPE_INT)
	{
		return INT2NUM(value_to_int(v));
	}
	else if (v_type == TYPE_LONG)
	{
		return LONG2NUM(value_to_long(v));
	}
	else if (v_type == TYPE_FLOAT)
	{
		return DBL2NUM((double)value_to_float(v));
	}
	else if (v_type == TYPE_DOUBLE)
	{
		return DBL2NUM(value_to_double(v));
	}
	else if (v_type == TYPE_STRING)
	{
		const char *str = value_to_string(v);

		return rb_str_new_cstr(str);
	}
	else if (v_type == TYPE_NULL)
	{
		return Qnil;
	}
	else
	{
		rb_raise(rb_eArgError, "Unsupported return type");

		return Qnil;
	}
}

function_return function_rb_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	loader_impl_rb_function rb_function = (loader_impl_rb_function)impl;

	signature s = function_signature(func);

	const size_t args_size = size;

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

			args_value[args_count] = rb_type_serialize(args[args_count]);

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

	value v = NULL;

	const char *v_type_name = rb_type_deserialize(result_value, &v);

	(void)v_type_name; /* Unused */

	return v;
}

function_return function_rb_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
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
	static struct function_interface_type rb_interface = {
		&function_rb_interface_create,
		&function_rb_interface_invoke,
		&function_rb_interface_await,
		&function_rb_interface_destroy
	};

	return &rb_interface;
}

int rb_object_interface_create(object obj, object_impl impl)
{
	(void)obj;

	loader_impl_rb_object rb_obj = impl;

	rb_obj->object = Qnil;
	rb_obj->object_class = Qnil;

	return 0;
}

value rb_object_interface_get(object obj, object_impl impl, const char *key)
{
	(void)obj;

	loader_impl_rb_object rb_object = (loader_impl_rb_object)impl;

	VALUE rb_val_object = rb_object->object;

	VALUE got = rb_iv_get(rb_val_object, key);

	VALUE exception = rb_errinfo();

	if (exception != Qnil)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Error getting object '%s' member '%s'", object_name(obj), key);

		rb_set_errinfo(Qnil);

		return NULL;
	}

	value result = NULL;
	rb_type_deserialize(got, &result);

	return result;
}

int rb_object_interface_set(object obj, object_impl impl, const char *key, value v)
{
	(void)obj;

	loader_impl_rb_object rb_object = (loader_impl_rb_object)impl;

	VALUE rb_val_object = rb_object->object;

	rb_iv_set(rb_val_object, key, rb_type_serialize(v));

	VALUE exception = rb_errinfo();

	if (exception != Qnil)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Error setting object '%s' member '%s'", object_name(obj), key);

		rb_set_errinfo(Qnil);

		return 1;
	}

	return 0;
}

value rb_object_interface_method_invoke(object obj, object_impl impl, const char *method_name, object_args args, size_t argc)
{
	(void)obj;

	loader_impl_rb_object obj_impl = (loader_impl_rb_object)impl;

	if (obj_impl == NULL || obj_impl->object == Qnil)
	{
		return NULL;
	}

	VALUE *argv = malloc(sizeof(VALUE) * argc);
	for (size_t i = 0; i < argc; i++)
	{
		argv[i] = rb_type_serialize(args[i]);
	}

	VALUE rb_retval = rb_funcallv(obj_impl->object, rb_intern(method_name), argc, argv);

	free(argv);

	if (rb_retval == Qnil)
	{
		return NULL;
	}

	value result = NULL;
	rb_type_deserialize(rb_retval, &result);

	return result;
}

value rb_object_interface_method_await(object obj, object_impl impl, const char *key, object_args args, size_t size, object_resolve_callback resolve, object_reject_callback reject, void *ctx)
{
	// TODO
	(void)obj;
	(void)impl;
	(void)key;
	(void)args;
	(void)size;
	(void)resolve;
	(void)reject;
	(void)ctx;

	return NULL;
}

int rb_object_interface_destructor(object obj, object_impl impl)
{
	(void)obj;
	(void)impl;

	return 0;
}

void rb_object_interface_destroy(object obj, object_impl impl)
{
	(void)obj;

	loader_impl_rb_object rb_object = (loader_impl_rb_object)impl;

	if (rb_object != NULL)
	{
		rb_object->object = Qnil;

		rb_object->object_class = Qnil;

		free(rb_object);
	}
}

object_interface rb_object_interface_singleton()
{
	static struct object_interface_type rb_object_interface = {
		&rb_object_interface_create,
		&rb_object_interface_get,
		&rb_object_interface_set,
		&rb_object_interface_method_invoke,
		&rb_object_interface_method_await,
		&rb_object_interface_destructor,
		&rb_object_interface_destroy
	};

	return &rb_object_interface;
}

int rb_class_interface_create(klass cls, class_impl impl)
{
	(void)cls;

	loader_impl_rb_class rb_cls = impl;

	rb_cls->class = Qnil;

	return 0;
}

object rb_class_interface_constructor(klass cls, class_impl impl, const char *name, class_args args, size_t argc)
{
	(void)cls;

	loader_impl_rb_class rb_cls = impl;

	loader_impl_rb_object rb_obj = malloc(sizeof(struct loader_impl_rb_object_type));

	object obj = object_create(name, rb_obj, &rb_object_interface_singleton, cls);

	/* Get loader implementation from class */
	rb_obj->impl = rb_cls->impl;

	VALUE *argv = malloc(sizeof(VALUE) * argc);
	for (size_t i = 0; i < argc; i++)
	{
		argv[i] = rb_type_serialize(args[i]);
	}

	VALUE rbval_object = rb_funcallv(rb_cls->class, rb_intern("new"), argc, argv);

	free(argv);

	rb_obj->object = rbval_object;
	rb_obj->object_class = rb_cls->class;

	return obj;
}

value rb_class_interface_static_get(klass cls, class_impl impl, attribute attr)
{
	(void)cls;

	loader_impl_rb_class rb_class = (loader_impl_rb_class)impl;

	char *attrname = attribute_name(attr);

	if (attrname == NULL)
	{
		return NULL;
	}

	VALUE rb_val_class = rb_class->class;

	VALUE got = rb_cv_get(rb_val_class, attrname);

	VALUE exception = rb_errinfo();

	if (exception != Qnil)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Error getting class '%s' member '%s'", class_name(cls), attrname);

		rb_set_errinfo(Qnil);

		return NULL;
	}

	value result = NULL;
	rb_type_deserialize(got, &result);

	return result;
}

int rb_class_interface_static_set(klass cls, class_impl impl, attribute attr, value v)
{
	(void)cls;

	loader_impl_rb_class rb_class = (loader_impl_rb_class)impl;

	char *attrname = attribute_name(attr);

	if (attrname == NULL)
	{
		return 1;
	}

	VALUE rb_val_class = rb_class->class;

	rb_cv_set(rb_val_class, attrname, rb_type_serialize(v));

	VALUE exception = rb_errinfo();

	if (exception != Qnil)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Error setting class '%s' member '%s'", class_name(cls), attrname);

		rb_set_errinfo(Qnil);

		return 1;
	}

	return 0;
}

value rb_class_interface_static_invoke(klass cls, class_impl impl, method m, class_args args, size_t argc)
{
	// TODO
	(void)cls;
	(void)impl;
	(void)args;

	loader_impl_rb_class cls_impl = (loader_impl_rb_class)impl;

	if (cls_impl == NULL || cls_impl->class == Qnil)
	{
		return NULL;
	}

	char *methodname = method_name(m);

	if (methodname == NULL)
	{
		return NULL;
	}

	VALUE *argv = malloc(sizeof(VALUE) * argc);
	for (size_t i = 0; i < argc; i++)
	{
		argv[i] = rb_type_serialize(args[i]);
	}

	VALUE rb_retval = rb_funcallv(cls_impl->class, rb_intern(methodname), argc, argv);

	free(argv);

	if (rb_retval == Qnil)
	{
		return NULL;
	}

	value result = NULL;
	rb_type_deserialize(rb_retval, &result);

	return result;
}

value rb_class_interface_static_await(klass cls, class_impl impl, method m, class_args args, size_t size, class_resolve_callback resolve, class_reject_callback reject, void *ctx)
{
	// TODO
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

void rb_class_interface_destroy(klass cls, class_impl impl)
{
	(void)cls;

	loader_impl_rb_class rb_class = (loader_impl_rb_class)impl;

	if (rb_class != NULL)
	{
		rb_class->class = Qnil;

		free(rb_class);
	}
}

class_interface rb_class_interface_singleton()
{
	static struct class_interface_type rb_class_interface = {
		&rb_class_interface_create,
		&rb_class_interface_constructor,
		&rb_class_interface_static_get,
		&rb_class_interface_static_set,
		&rb_class_interface_static_invoke,
		&rb_class_interface_static_await,
		&rb_class_interface_destroy
	};

	return &rb_class_interface;
}

int rb_loader_impl_initialize_types(loader_impl impl)
{
	/* TODO: move this to loader_impl by passing the structure and loader_impl_derived callback */

	static struct
	{
		type_id id;
		const char *name;
	} type_id_name_pair[] = {
		{ TYPE_BOOL, "Boolean" },
		{ TYPE_INT, "Fixnum" },
		{ TYPE_LONG, "Bignum" },
		{ TYPE_DOUBLE, "Float" },
		{ TYPE_STRING, "String" },
		{ TYPE_ARRAY, "Array" },
		{ TYPE_NULL, "NilClass" },
		{ TYPE_CLASS, "Class" },
		{ TYPE_OBJECT, "Object" }
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

loader_impl_data rb_loader_impl_initialize(loader_impl impl, configuration config)
{
	static struct rb_loader_impl_type
	{
		void *unused;

	} rb_loader_impl_unused = {
		NULL
	};

	(void)impl;
	(void)config;

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

		/* Gem add home folder if any */
		/*
		{
			const char * gem_home_env = getenv("GEM_HOME");

			if (gem_home_env != NULL)
			{
				if (rb_loader_impl_execution_path(impl, gem_home_env) != 0)
				{
					log_write("metacall", LOG_LEVEL_WARNING, "Ruby GEM_HOME could not be added to execution path list");
				}
			}
		}
		*/

		log_write("metacall", LOG_LEVEL_DEBUG, "Ruby loader initialized correctly");
	}

	/* Register initialization */
	loader_initialization_register(impl);

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

		VALUE module_absolute_path = rb_funcall(rb_cFile, rb_intern("join"), 2, load_path_entry, module_path);

		module = rb_loader_impl_load_data_absolute(module_absolute_path);

		if (module != Qnil)
		{
			return module;
		}
	}

	return Qnil;
}

VALUE rb_loader_impl_module_eval_protect(VALUE args)
{
	/* TODO: Do this properly */
	loader_impl_rb_module_eval_protect protect = (loader_impl_rb_module_eval_protect)args;

	return rb_mod_module_eval(protect->argc, protect->argv, protect->module);
}

VALUE rb_loader_impl_module_eval(VALUE module, VALUE module_data)
{
	const int argc = 1;
	VALUE result;
	VALUE argv[argc];
	struct loader_impl_rb_module_eval_protect_type protect;
	int state;

	argv[0] = module_data;

	protect.argc = argc;
	protect.argv = argv;
	protect.module = module;

	result = rb_protect(rb_loader_impl_module_eval_protect, (VALUE)&protect, &state);

	if (state || result == Qnil)
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
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Ruby module backtrace not available");
		}

		return Qnil;
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
						rb_loader_impl_key_clear(rb_module->function_map);

						free(rb_module);

						return NULL;
					}

					log_write("metacall", LOG_LEVEL_DEBUG, "Ruby module %s loaded", path);

					rb_loader_impl_key_print(rb_module->function_map);

					return rb_module;
				}
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

		loader_path_get_module_name(paths[iterator], module_name, "rb");

		rb_module = rb_loader_impl_load_from_file_module(impl, paths[iterator], module_name);

		if (rb_module == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid ruby module loading %s", paths[iterator]);
		}
		else
		{
			vector_push_back(handle->modules, &rb_module);
		}
	}

	// Do not load the handle in case there isn't modules
	if (vector_size(handle->modules) == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "No module could be loaded");

		vector_destroy(handle->modules);

		free(handle);

		return NULL;
	}

	return (loader_handle)handle;
}

loader_impl_rb_module rb_loader_impl_load_from_memory_module(loader_impl impl, const loader_naming_name name, const char *buffer, size_t size)
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
		}
	}

	return NULL;
}

loader_handle rb_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char *buffer, size_t size)
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
	VALUE *module = (VALUE *)args;

	rb_function_parser function_parser = (rb_function_parser)val;

	VALUE name = rb_str_new_cstr(function_parser->name);

	(void)s;
	(void)key;

	/* TODO: This generates a segmentation fault in metacall-ruby-object-class-test */
	/* rb_undef(*module, rb_to_id(name)); */

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
		loader_impl_rb_module *rb_module = vector_at(rb_handle->modules, iterator);

		/* Undef all methods */
		set_iterate((*rb_module)->function_map, &rb_loader_impl_clear_cb_iterate, (set_cb_iterate_args) & ((*rb_module)->module));

		/* Remove module */
		if (rb_is_const_id((*rb_module)->id))
		{
			VALUE result = rb_const_remove(rb_cObject, (*rb_module)->id);

			/* TODO: Handle result */
			(void)result;
		}

		rb_loader_impl_key_clear((*rb_module)->function_map);
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
	log_write("metacall", LOG_LEVEL_DEBUG, "Ruby loader discovering:");

	VALUE instance_methods = rb_funcall(rb_module->module, rb_intern("instance_methods"), 0);

	VALUE methods_size = rb_funcall(instance_methods, rb_intern("size"), 0);

	int index, size = FIX2INT(methods_size);

	for (index = 0; index < size; ++index)
	{
		VALUE method = rb_ary_entry(instance_methods, index);

		if (method != Qnil)
		{
			VALUE method_name = rb_funcall(method, rb_intern("id2name"), 0);

			const char *method_name_str = RSTRING_PTR(method_name);

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

					scope_define(sp, function_name(f), value_create_function(f));

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

	/* Now discover classes */
	VALUE constants = rb_funcall(rb_module->module, rb_intern("constants"), 0);

	VALUE constants_size = rb_funcall(constants, rb_intern("size"), 0);

	size = FIX2INT(constants_size);

	for (index = 0; index < size; index++)
	{
		VALUE constant = rb_ary_entry(constants, index);

		if (constant != Qnil)
		{
			if (RB_TYPE_P(constant, T_SYMBOL))
			{
				VALUE class_name = rb_funcall(constant, rb_intern("id2name"), 0);
				const char *class_name_str = RSTRING_PTR(class_name);

				log_write("metacall", LOG_LEVEL_DEBUG, "Class name %s", class_name_str);

				VALUE class = rb_const_get_from(rb_module->module, rb_intern(class_name_str));

				// rb_obj_private_methods, rb_obj_protected_methods, rb_obj_public_methods and
				// rb_obj_singleton_methods, can be used instead of rb_class_instance_methods

				/*
				VALUE argv[1] = { Qtrue }; // include_superclasses ? Qtrue : Qfalse;
   				VALUE methods = rb_class_instance_methods(1, argv, class); // argc, argv, class
				VALUE load_path_array_size = rb_funcall(methods, rb_intern("size"), 0);
				int method_index, methods_size = FIX2INT(load_path_array_size);

				for (method_index = 0; method_index < methods_size; method_index++)
				{
					VALUE method = rb_ary_entry(methods, method_index);
					VALUE method_name = rb_funcall(method, rb_intern("id2name"), 0);
					const char * method_name_str = RSTRING_PTR(method_name);

					log_write("metacall", LOG_LEVEL_DEBUG, "Method inside '%s' %s", class_name_str, method_name_str);
				}
				*/

				loader_impl_rb_class rb_cls = malloc(sizeof(struct loader_impl_rb_class_type));

				klass c = class_create(class_name_str, rb_cls, &rb_class_interface_singleton);

				rb_cls->impl = impl;
				rb_cls->class = class;

				scope sp = context_scope(ctx);
				scope_define(sp, class_name_str, value_create_class(c));
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
		loader_impl_rb_module *rb_module = vector_at(rb_handle->modules, iterator);

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

	/* Destroy children loaders */
	loader_unload_children(impl);

	return ruby_cleanup(0);
}
