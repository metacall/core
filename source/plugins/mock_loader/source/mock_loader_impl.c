/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading mock code at run-time into a process.
 *
 */

#include <mock_loader/mock_loader_impl.h>

#include <loader/loader_impl.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

#include <log/log.h>

#include <stdlib.h>

typedef struct loader_impl_mock_type
{
	void * impl_mock_data;

} * loader_impl_mock;

typedef struct loader_impl_mock_handle_type
{
	void * handle_mock_data;

} * loader_impl_mock_handle;


typedef struct loader_impl_mock_function_type
{
	loader_impl_mock_handle mock_handle;
	void * function_mock_data;

} * loader_impl_mock_function;

int function_mock_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

function_return function_mock_interface_invoke(function func, function_impl impl, function_args args)
{
	loader_impl_mock_function mock_function = (loader_impl_mock_function)impl;

	signature s = function_signature(func);

	type ret_type = signature_get_return(s);

	const size_t args_size = signature_count(s);

	(void)mock_function;

	if (args_size > 0)
	{
		size_t args_count;

		for (args_count = 0; args_count < args_size; ++args_count)
		{
			type t = signature_get_type(s, args_count);

			type_id id = type_index(t);

			log_write("metacall", LOG_LEVEL_DEBUG, "Type %p, %d", (void *)t, id);

			if (id == TYPE_BOOL)
			{
				boolean * value_ptr = (boolean *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Boolean value: %d", *value_ptr);
			}
			else if (id == TYPE_CHAR)
			{
				char * value_ptr = (char *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Char value: %c", *value_ptr);
			}
			else if (id == TYPE_INT)
			{
				int * value_ptr = (int *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Int value: %d", *value_ptr);
			}
			else if (id == TYPE_LONG)
			{
				long * value_ptr = (long *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Long value: %ld", *value_ptr);
			}
			else if (id == TYPE_DOUBLE)
			{
				double * value_ptr = (double *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Double value: %f", *value_ptr);
			}
			else if (id == TYPE_STRING)
			{
				const char * value_ptr = (const char *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "String value: %s", value_ptr);
			}
			else if (id == TYPE_PTR)
			{
				void * value_ptr = (void *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Pointer value: %p", value_ptr);
			}
			else
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Unrecognized value: %p", args[args_count]);
			}
		}

		log_write("metacall", LOG_LEVEL_DEBUG, "Calling mock function with arguments (%lu)", args_size);
	}
	else
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "Calling mock function without arguments");
	}

	if (ret_type != NULL)
	{
		type_id id = type_index(ret_type);

		log_write("metacall", LOG_LEVEL_DEBUG, "Return type %p, %d", (void *)ret_type, id);

		if (id == TYPE_BOOL)
		{
			boolean b = 1;

			return value_create_bool(b);
		}
		else if (id == TYPE_CHAR)
		{
			return value_create_char('A');
		}
		else if (id == TYPE_INT)
		{
			return value_create_int(1234);
		}
		else if (id == TYPE_LONG)
		{
			return value_create_long(90000L);
		}
		else if (id == TYPE_DOUBLE)
		{
			return value_create_double(3.1416);
		}
		else if (id == TYPE_STRING)
		{
			static const char str[] = "Hello World";

			return value_create_string(str, sizeof(str) / sizeof(str[0]));
		}
		else if (id == TYPE_PTR)
		{
			static int int_val = 15;

			return value_create_ptr(&int_val);
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Unrecognized return type");
		}
	}

	return NULL;
}

void function_mock_interface_destroy(function func, function_impl impl)
{
	loader_impl_mock_function mock_function = (loader_impl_mock_function)impl;

	(void)func;

	if (mock_function != NULL)
	{
		free(mock_function);
	}
}

function_interface function_mock_singleton(void)
{
	static struct function_interface_type mock_interface =
	{
		&function_mock_interface_create,
		&function_mock_interface_invoke,
		&function_mock_interface_destroy
	};

	return &mock_interface;
}

int mock_loader_impl_initialize_types(loader_impl impl)
{
	/* TODO: move this to loader_impl by passing the structure and loader_impl_derived callback */

	static struct
	{
		type_id id;
		const char * name;
	}
	type_id_name_pair[] =
	{
		{ TYPE_BOOL,	"Boolean"	},
		{ TYPE_CHAR,	"Char"		},
		{ TYPE_INT,	"Integer"	},
		{ TYPE_LONG,	"Long"		},
		{ TYPE_DOUBLE,	"Double"	},
		{ TYPE_STRING,	"String"	},
		{ TYPE_PTR,	"Ptr"		}
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

loader_impl_data mock_loader_impl_initialize(loader_impl impl)
{
	loader_impl_mock mock_impl = malloc(sizeof(struct loader_impl_mock_type));

	(void)impl;

	if (mock_impl != NULL)
	{
		if (mock_loader_impl_initialize_types(impl) == 0)
		{
			mock_impl->impl_mock_data = NULL;

			return mock_impl;
		}

		free(mock_impl);
	}

	return NULL;
}

int mock_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	(void)impl;
	(void)path;

	return 0;
}

loader_handle mock_loader_impl_load(loader_impl impl, const loader_naming_path path, loader_naming_name name)
{
	loader_impl_mock_handle handle = malloc(sizeof(struct loader_impl_mock_handle_type));

	(void)impl;

	if (handle != NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "Mock module %s (%s) loaded", path, name);

		handle->handle_mock_data = NULL;

		return (loader_handle)handle;
	}

	return NULL;
}

int mock_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_mock_handle mock_handle = (loader_impl_mock_handle)handle;

	(void)impl;

	if (mock_handle != NULL)
	{
		free(mock_handle);

		return 0;
	}

	return 1;
}

loader_impl_mock_function mock_function_create(loader_impl_mock_handle mock_handle)
{
	loader_impl_mock_function mock_function = malloc(sizeof(struct loader_impl_mock_function_type));

	if (mock_function != NULL)
	{
		mock_function->mock_handle = mock_handle;

		mock_function->function_mock_data = NULL;

		return mock_function;
	}

	return NULL;
}

int mock_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_mock mock_impl = loader_impl_get(impl);

	loader_impl_mock_handle mock_handle = (loader_impl_mock_handle)handle;

	loader_impl_mock_function mock_function = mock_function_create(mock_handle);

	scope sp = context_scope(ctx);

	(void)mock_impl;

	if (mock_function != NULL)
	{
		function f = function_create("my_empty_func", 0, mock_function, &function_mock_singleton);

		signature s = function_signature(f);

		signature_set_return(s, loader_impl_type(impl, "Integer"));

		scope_define(sp, function_name(f), f);
	}

	mock_function = mock_function_create(mock_handle);

	if (mock_function != NULL)
	{
		function f = function_create("two_doubles", 2, mock_function, &function_mock_singleton);

		signature s = function_signature(f);

		signature_set_return(s, loader_impl_type(impl, "Double"));

		signature_set(s, 0, "first_parameter", loader_impl_type(impl, "Double"));

		signature_set(s, 1, "second_parameter", loader_impl_type(impl, "Double"));

		scope_define(sp, function_name(f), f);
	}

	mock_function = mock_function_create(mock_handle);

	if (mock_function != NULL)
	{
		function f = function_create("mixed_args", 5, mock_function, &function_mock_singleton);

		signature s = function_signature(f);

		signature_set_return(s, loader_impl_type(impl, "Char"));

		signature_set(s, 0, "a_char", loader_impl_type(impl, "Char"));

		signature_set(s, 1, "b_int", loader_impl_type(impl, "Integer"));

		signature_set(s, 2, "c_long", loader_impl_type(impl, "Long"));

		signature_set(s, 3, "d_double", loader_impl_type(impl, "Double"));

		signature_set(s, 4, "e_ptr", loader_impl_type(impl, "Ptr"));

		scope_define(sp, function_name(f), f);
	}

	mock_function = mock_function_create(mock_handle);

	if (mock_function != NULL)
	{
		function f = function_create("new_args", 1, mock_function, &function_mock_singleton);

		signature s = function_signature(f);

		signature_set_return(s, loader_impl_type(impl, "String"));

		signature_set(s, 0, "a_str", loader_impl_type(impl, "String"));

		scope_define(sp, function_name(f), f);
	}

	mock_function = mock_function_create(mock_handle);

	if (mock_function != NULL)
	{
		function f = function_create("two_str", 2, mock_function, &function_mock_singleton);

		signature s = function_signature(f);

		signature_set_return(s, loader_impl_type(impl, "String"));

		signature_set(s, 0, "a_str", loader_impl_type(impl, "String"));

		signature_set(s, 1, "b_str", loader_impl_type(impl, "String"));

		scope_define(sp, function_name(f), f);
	}

	mock_function = mock_function_create(mock_handle);

	if (mock_function != NULL)
	{
		function f = function_create("three_str", 3, mock_function, &function_mock_singleton);

		signature s = function_signature(f);

		signature_set_return(s, loader_impl_type(impl, "String"));

		signature_set(s, 0, "a_str", loader_impl_type(impl, "String"));

		signature_set(s, 1, "b_str", loader_impl_type(impl, "String"));

		signature_set(s, 2, "c_str", loader_impl_type(impl, "String"));

		scope_define(sp, function_name(f), f);
	}

	return 0;
}

int mock_loader_impl_destroy(loader_impl impl)
{
	loader_impl_mock mock_impl = loader_impl_get(impl);

	if (mock_impl != NULL)
	{
		free(mock_impl);

		return 0;
	}

	return 1;
}
