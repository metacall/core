/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

static value function_metadata_name(function func);

static value function_metadata_signature(function func);

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

value function_metadata_name(function func)
{
	static const char func_str[] = "name";

	value name = value_create_array(NULL, 2);

	value * name_array;

	if (name == NULL)
	{
		return NULL;
	}

	name_array = value_to_array(name);

	name_array[0] = value_create_string(func_str, sizeof(func_str) - 1);

	if (name_array[0] == NULL)
	{
		value_type_destroy(name);

		return NULL;
	}

	name_array[1] = value_create_string(func->name, strlen(func->name));

	if (name_array[1] == NULL)
	{
		value_type_destroy(name);

		return NULL;
	}

	return name;
}

value function_metadata_signature(function func)
{
	static const char sig_str[] = "signature";

	value sig = value_create_array(NULL, 2);

	value * sig_array;

	if (sig == NULL)
	{
		return NULL;
	}

	sig_array = value_to_array(sig);

	sig_array[0] = value_create_string(sig_str, sizeof(sig_str) - 1);

	if (sig_array[0] == NULL)
	{
		value_type_destroy(sig);

		return NULL;
	}

	sig_array[1] = signature_metadata(func->s);

	if (sig_array[1] == NULL)
	{
		value_type_destroy(sig);

		return NULL;
	}

	return sig;
}

value function_metadata(function func)
{
	value name, sig, f;

	value * f_map;

	/* Create function name array */
	name = function_metadata_name(func);

	if (name == NULL)
	{
		return NULL;
	}

	/* Create signature array */
	sig = function_metadata_signature(func);

	if (sig == NULL)
	{
		value_type_destroy(name);

		return NULL;
	}

	/* Create function map (name + signature) */
	f = value_create_map(NULL, 2);

	if (f == NULL)
	{
		value_type_destroy(name);

		value_type_destroy(sig);

		return NULL;
	}

	f_map = value_to_map(f);

	f_map[0] = name;

	f_map[1] = sig;

	return f;
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
