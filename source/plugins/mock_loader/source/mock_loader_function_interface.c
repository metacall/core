/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading mock code at run-time into a process.
 *
 */

#include <mock_loader/mock_loader_function_interface.h>

#include <reflect/type.h>
#include <reflect/scope.h>
#include <reflect/context.h>

#include <stdio.h>

function_interface function_mock_singleton(void);

int function_mock_interface_create(function func, function_impl impl);

function_return function_mock_interface_invoke(function func, function_impl impl, function_args args);

void function_mock_interface_destroy(function func, function_impl impl);

function_impl_interface_singleton mock_loader_impl_function_interface()
{
	return &function_mock_singleton;
}

function_interface function_mock_singleton()
{
	static struct function_interface_type mock_interface =
	{
		&function_mock_interface_create,
		&function_mock_interface_invoke,
		&function_mock_interface_destroy
	};

	return &mock_interface;
}

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

			printf("Type %p, %d\n", (void *)t, id);

			if (id == TYPE_BOOL)
			{
				boolean * b = (boolean *)(args[args_count]);

				printf("Boolean value: %d\n", *b);
			}
			else if (id == TYPE_CHAR)
			{
				char * value_ptr = (char *)(args[args_count]);

				printf("Char value: %c\n", *value_ptr);
			}
			else if (id == TYPE_INT)
			{
				int * value_ptr = (int *)(args[args_count]);

				printf("Int value: %d\n", *value_ptr);
			}
			else if (id == TYPE_LONG)
			{
				long * value_ptr = (long *)(args[args_count]);

				printf("Long value: %ld\n", *value_ptr);
			}
			else if (id == TYPE_DOUBLE)
			{
				double * value_ptr = (double *)(args[args_count]);

				printf("Double value: %f\n", *value_ptr);
			}
			else if (id == TYPE_STRING)
			{
				const char * value_ptr = (const char *)(args[args_count]);

				printf("String value: %s\n", value_ptr);
			}
			else if (id == TYPE_PTR)
			{
				void * value_ptr = (void *)(args[args_count]);

				printf("Pointer value: %p\n", value_ptr);
			}
			else
			{
				printf("Unrecognized value: %p\n", args[args_count]);
			}
		}

		printf("Calling mock function with arguments (%lu)\n", args_size);
	}
	else
	{
		printf("Calling mock function without arguments\n");
	}

	if (ret_type != NULL)
	{
		type_id id = type_index(ret_type);

		printf("Return type %p, %d\n", (void *)ret_type, id);

		if (id == TYPE_BOOL)
		{
			return value_create_bool(1);
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

			return value_create_string(str, sizeof(str));
		}
		else if (id == TYPE_PTR)
		{
			static int int_val = 15;

			return value_create_ptr(&int_val);
		}
		else
		{
			printf("Unrecognized return type\n");
		}
	}

	return NULL;
}

void function_mock_interface_destroy(function func, function_impl impl)
{
	(void)func;
	(void)impl;
}
