/*
 *	Loader Library by Parra Studios
 *	A plugin for loading mock code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <mock_loader/mock_loader_function_interface.h>

#include <reflect/type.h>
#include <reflect/scope.h>
#include <reflect/context.h>

#include <log/log.h>

function_interface function_mock_singleton(void);

int function_mock_interface_create(function func, function_impl impl);

function_return function_mock_interface_invoke(function func, function_impl impl, function_args args, size_t size);

void function_mock_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void * context);

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
		&function_mock_interface_await,
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

function_return function_mock_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	loader_impl_mock_function mock_function = (loader_impl_mock_function)impl;

	signature s = function_signature(func);

	type ret_type = signature_get_return(s);

	(void)mock_function;

	if (size > 0)
	{
		size_t args_count;

		for (args_count = 0; args_count < size; ++args_count)
		{
			type t = signature_get_type(s, args_count);

			type_id id = type_index(t);

			log_write("metacall", LOG_LEVEL_DEBUG, "Type %p, %d", (void *)t, id);

			if (id == TYPE_BOOL)
			{
				boolean * b = (boolean *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Boolean value: %d", *b);
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
			else if (id == TYPE_FLOAT)
			{
				float * value_ptr = (float *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Float value: %f", *value_ptr);
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

		log_write("metacall", LOG_LEVEL_DEBUG, "Calling mock function with arguments (%lu)", size);
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
		else if (id == TYPE_FLOAT)
		{
			return value_create_float(0.2f);
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
			log_write("metacall", LOG_LEVEL_ERROR, "Unrecognized return type");
		}
	}

	return NULL;
}

function_return function_mock_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void * context)
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

void function_mock_interface_destroy(function func, function_impl impl)
{
	(void)func;
	(void)impl;
}
