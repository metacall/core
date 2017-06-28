/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <reflect/reflect_function.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

struct function_type
{
	char * name;
	signature s;
	function_impl impl;
	function_interface interface;
};

function function_create(const char * name, size_t args_count, function_impl impl, function_impl_interface_singleton singleton)
{
	if (name != NULL)
	{
		function func = malloc(sizeof(struct function_type));

		if (func != NULL)
		{
			size_t func_name_size = strlen(name) + 1;

			func->name = malloc(sizeof(char) * func_name_size);

			if (func->name == NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid function name allocation <%s>", name);

				free(func);

				return NULL;
			}

			memcpy(func->name, name, func_name_size);

			func->impl = impl;

			func->s = signature_create(args_count);

			if (func->s != NULL)
			{
				if (singleton)
				{
					func->interface = singleton();
				}
				else
				{
					func->interface = NULL;
				}

				if (func->interface != NULL && func->interface->create != NULL)
				{
					if (func->interface->create(func, impl) != 0)
					{
						log_write("metacall", LOG_LEVEL_ERROR, "Invalid function (%s) create callback <%p>", func->name, func->interface->create);
					}
				}

				return func;
			}

			free(func);
		}
	}

	return NULL;
}

const char * function_name(function func)
{
	if (func != NULL)
	{
		return func->name;
	}

	return NULL;
}

signature function_signature(function func)
{
	if (func != NULL)
	{
		return func->s;
	}

	return NULL;
}

void function_print(function func)
{
	if (func != NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "Function <%s> Impl (%p) Interface (%p):",
			func->name, (void *)func->impl, (void *)func->interface);

		signature_print(func->s);
	}
}

char * function_dump(function func, size_t * size)
{
	if (func != NULL)
	{
		char * buffer = NULL;

		size_t func_name_length = strlen(func->name);

		size_t index, copy_length = 0, length = func_name_length;

		type ret = signature_get_return(func->s);

		size_t args_count = signature_count(func->s);

		const char * ret_name = type_name(ret);

		size_t ret_name_length = (ret_name == NULL) ? 0 : strlen(ret_name);

		size_t duck_type_count = 0;

		length += ret_name_length;

		for (index = 0; index < args_count; ++index)
		{
			const char * arg_name = signature_get_name(func->s, index);

			type t = signature_get_type(func->s, index);

			length += strlen(arg_name);

			if (t != NULL)
			{
				length += strlen(type_name(t));
			}
			else
			{
				++duck_type_count;
			}
		}

		length += (args_count > 0) ? (3 * args_count) - duck_type_count : 2;

		if (ret_name != NULL)
		{
			++length;
		}

		buffer = malloc((length + 1) * sizeof(char));
		
		if (buffer == NULL)
		{
			return NULL;
		}

		if (ret_name != NULL)
		{
			memcpy(buffer, ret_name, ret_name_length);

			buffer[ret_name_length] = ' ';

			copy_length += ret_name_length + 1;
		}

		memcpy(&buffer[copy_length], func->name, func_name_length);

		copy_length += func_name_length;

		buffer[copy_length++] = '(';

		for (index = 0; index < args_count; ++index)
		{
			const char * arg_name = signature_get_name(func->s, index);

			type t = signature_get_type(func->s, index);

			size_t arg_name_length = strlen(arg_name);

			memcpy(&buffer[copy_length], arg_name, arg_name_length);

			copy_length += arg_name_length;

			if (t != NULL)
			{
				size_t arg_type_name_length = strlen(type_name(t));

				buffer[copy_length++] = ' ';

				memcpy(&buffer[copy_length], type_name(t), arg_type_name_length);

				copy_length += arg_type_name_length;
			}

			if (index < args_count - 1)
			{
				buffer[copy_length++] = ',';
				buffer[copy_length++] = ' ';
			}
		}

		buffer[copy_length++] = ')';
		buffer[copy_length] = '\0';

		if (copy_length != length)
		{
			free(buffer);

			return NULL;
		}

		*size = copy_length + 1;

		return buffer;
	}

	return NULL;
}

function_return function_call(function func, function_args args)
{
	if (func != NULL && args != NULL)
	{
		if (func->interface != NULL && func->interface->invoke != NULL)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Invoke function (%s) with args <%p>", func->name, (void *)args);

			return func->interface->invoke(func, func->impl, args);
		}
	}

	return NULL;
}

void function_destroy(function func)
{
	if (func != NULL)
	{
		if (func->interface != NULL && func->interface->destroy != NULL)
		{
			func->interface->destroy(func, func->impl);
		}

		signature_destroy(func->s);

		free(func->name);

		free(func);
	}
}
