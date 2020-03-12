/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <reflect/reflect_function.h>
#include <reflect/reflect_value_type.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

struct function_type
{
	char * name;
	signature s;
	function_impl impl;
	function_interface interface;
	size_t ref_count;
	void * data;
};

static value function_metadata_name(function func);

static value function_metadata_signature(function func);

function function_create(const char * name, size_t args_count, function_impl impl, function_impl_interface_singleton singleton)
{
	function func = malloc(sizeof(struct function_type));

	if (func == NULL)
	{
		return NULL;
	}

	if (name != NULL)
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
	}
	else
	{
		func->name = NULL;
	}

	func->impl = impl;
	func->ref_count = 0;
	func->data = NULL;

	func->s = signature_create(args_count);

	if (func->s == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid function signature allocation");

		free(func->name);
		free(func);

		return NULL;
	}

	func->interface = singleton ? singleton() : NULL;

	if (func->interface != NULL && func->interface->create != NULL)
	{
		if (func->interface->create(func, impl) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid function (%s) create callback <%p>", func->name, func->interface->create);

			free(func->name);
			free(func);

			return NULL;
		}
	}

	return func;
}

int function_resize(function func, size_t count)
{
	signature new_s;

	if (func == NULL)
	{
		return 1;
	}

	new_s = signature_resize(func->s, count);

	if (new_s == NULL)
	{
		return 1;
	}

	func->s = new_s;

	return 0;
}

int function_increment_reference(function func)
{
	if (func == NULL)
	{
		return 1;
	}

	if (func->ref_count == SIZE_MAX)
	{
		return 1;
	}

	++func->ref_count;

	return 0;
}

int function_decrement_reference(function func)
{
	if (func == NULL)
	{
		return 1;
	}

	if (func->ref_count == 0)
	{
		return 1;
	}

	--func->ref_count;

	return 0;
}

void function_bind(function func, void * data)
{
	func->data = data;
}

void * function_closure(function func)
{
	if (func != NULL)
	{
		return func->data;
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
			if (func->name == NULL)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Invoke annonymous function with args <%p>", (void *)args);
			}
			else
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Invoke function (%s) with args <%p>", func->name, (void *)args);
			}

			return func->interface->invoke(func, func->impl, args);
		}
	}

	return NULL;
}

function_return function_await(function func, function_args args, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void * context)
{
	if (func != NULL && args != NULL)
	{
		if (func->interface != NULL && func->interface->await != NULL)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Await function (%s) with args <%p>", func->name, (void *)args);

			return func->interface->await(func, func->impl, args, resolve_callback, reject_callback, context);
		}
	}

	return NULL;
}

void function_destroy(function func)
{
	if (func != NULL && func->ref_count == 0)
	{
		if (func->interface != NULL && func->interface->destroy != NULL)
		{
			func->interface->destroy(func, func->impl);
		}

		signature_destroy(func->s);

		if (func->name != NULL)
		{
			free(func->name);
		}

		free(func);
	}
}
