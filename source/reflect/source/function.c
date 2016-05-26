/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <reflect/function.h>

#include <string.h>

typedef struct function_type
{
	char * name;
	signature s;
	function_impl impl;
	function_interface interface;
} * function;

function function_create(const char * name, size_t args_count, function_impl impl, function_impl_interface_singleton singleton)
{
	if (name != NULL)
	{
		function func = malloc(sizeof(struct function_type));

		if (func != NULL)
		{
			func->name = strdup(name);

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
						/* error */
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

void function_call(function func, function_args args)
{
	if (func != NULL && args != NULL)
	{
		if (func->interface != NULL && func->interface->invoke != NULL)
		{
			func->interface->invoke(func, func->impl, args);
		}
	}
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
