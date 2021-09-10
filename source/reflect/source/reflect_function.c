/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#include <reflect/reflect_function.h>
#include <reflect/reflect_value_type.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

struct function_type
{
	char *name;
	signature s;
	function_impl impl;
	function_interface interface;
	size_t ref_count;
	enum async_id async;
	void *data;
};

static struct
{
	uint64_t allocations;
	uint64_t deallocations;
	uint64_t increments;
	uint64_t decrements;
} function_stats = { 0, 0, 0, 0 };

static value function_metadata_name(function func);
static value function_metadata_async(function func);
static value function_metadata_signature(function func);

function function_create(const char *name, size_t args_count, function_impl impl, function_impl_interface_singleton singleton)
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
	func->async = SYNCHRONOUS;
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

	++function_stats.allocations;

	return func;
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
	++function_stats.increments;

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
	++function_stats.decrements;

	return 0;
}

void function_async(function func, enum async_id async)
{
	func->async = async;
}

enum async_id function_async_id(function func)
{
	return func->async;
}

void function_bind(function func, void *data)
{
	func->data = data;
}

void *function_closure(function func)
{
	if (func != NULL)
	{
		return func->data;
	}

	return NULL;
}

const char *function_name(function func)
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

	value *name_array;

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

value function_metadata_async(function func)
{
	static const char async_str[] = "async";

	value async = value_create_array(NULL, 2);

	value *async_array;

	if (async == NULL)
	{
		return NULL;
	}

	async_array = value_to_array(async);

	async_array[0] = value_create_string(async_str, sizeof(async_str) - 1);

	if (async_array[0] == NULL)
	{
		value_type_destroy(async);

		return NULL;
	}

	async_array[1] = value_create_bool(func->async == SYNCHRONOUS ? 0L : 1L);

	if (async_array[1] == NULL)
	{
		value_type_destroy(async);

		return NULL;
	}

	return async;
}

value function_metadata_signature(function func)
{
	static const char sig_str[] = "signature";

	value sig = value_create_array(NULL, 2);

	value *sig_array;

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
	value name, sig, async, f;
	value *f_map;

	/* Create function name array */
	name = function_metadata_name(func);

	if (name == NULL)
	{
		goto error_name;
	}

	/* Create signature array */
	sig = function_metadata_signature(func);

	if (sig == NULL)
	{
		goto error_signature;
	}

	/* Create function async array */
	async = function_metadata_async(func);

	if (async == NULL)
	{
		goto error_async;
	}

	/* Create function map (name + signature + async) */
	f = value_create_map(NULL, 3);

	if (f == NULL)
	{
		goto error_function;
	}

	f_map = value_to_map(f);

	f_map[0] = name;
	f_map[1] = sig;
	f_map[2] = async;

	return f;

error_function:
	value_type_destroy(async);
error_async:
	value_type_destroy(sig);
error_signature:
	value_type_destroy(name);
error_name:
	return NULL;
}

/* TODO: Implement a complete new module for debugging and tracking the values */

#if 0 /* (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__)) */
static void function_call_value_debug(type_id id, value v)
{
	switch (id)
	{
		case TYPE_BOOL :
		{
			boolean b = value_to_bool(v);
			if (b == 0L)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "    => False");
			}
			else if (b == 1L)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "    => True");
			}
			else
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "    => Invalid");
			}
			break;
		}

		case TYPE_CHAR :
		{
			char c = value_to_char(v);
			log_write("metacall", LOG_LEVEL_DEBUG, "    => %c", c);
			break;
		}

		case TYPE_SHORT :
		{
			short s = value_to_short(v);
			log_write("metacall", LOG_LEVEL_DEBUG, "    => %d", s);
			break;
		}

		case TYPE_INT :
		{
			int i = value_to_int(v);
			log_write("metacall", LOG_LEVEL_DEBUG, "    => %d", i);
			break;
		}

		case TYPE_LONG :
		{
			long l = value_to_long(v);
			log_write("metacall", LOG_LEVEL_DEBUG, "    => %d", l);
			break;
		}

		case TYPE_FLOAT :
		{
			float f = value_to_float(v);
			log_write("metacall", LOG_LEVEL_DEBUG, "    => %f", f);
			break;
		}

		case TYPE_DOUBLE :
		{
			double d = value_to_double(v);
			log_write("metacall", LOG_LEVEL_DEBUG, "    => %f", d);
			break;
		}

		case TYPE_STRING :
		{
			char * str = value_to_string(v);
			log_write("metacall", LOG_LEVEL_DEBUG, "    => %s", str);
			break;
		}

		case TYPE_BUFFER :
		{
			char * buf = value_to_buffer(v);
			log_write("metacall", LOG_LEVEL_DEBUG, "    => %.*s", value_type_size(v), buf);
			break;
		}

		case TYPE_ARRAY :
		{
			value * v_array = value_to_array(v);
			log_write("metacall", LOG_LEVEL_DEBUG, "    => %p", (void *)v_array);
			/* TODO: Recursive */
			break;
		}

		case TYPE_MAP :
		{
			value * v_map = value_to_map(v);
			log_write("metacall", LOG_LEVEL_DEBUG, "    => %p", (void *)v_map);
			/* TODO: Recursive */
			break;
		}

		case TYPE_PTR :
		{
			void * ptr = value_to_ptr(v);
			log_write("metacall", LOG_LEVEL_DEBUG, "    => %p", ptr);
			break;
		}

		case TYPE_FUTURE :
		{
			future f = value_to_future(v);
			log_write("metacall", LOG_LEVEL_DEBUG, "    => %p", (void *)f); /* TODO */
			break;
		}

		case TYPE_FUNCTION :
		{
			function f = value_to_function(v);
			if (f->name != NULL)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "    => <%p> function %s with arity %u",
					(void *)f, f->name, signature_count(f->s));
			}
			else
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "    => <%p> function anonymous with arity %u",
					(void *)f, signature_count(f->s));
			}
			/* TODO: Recursive */
			break;
		}

		case TYPE_NULL :
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "    => NULL");
			break;
		}

		default :
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "    => Invalid");
			break;
		}
	}
}

static function_return function_call_debug(function func, function_args args, size_t size)
{
	size_t iterator;
	value ret;
	type_id id_ret = TYPE_INVALID, id_ret_sig;
	type ret_type;

	if (func->name == NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "Invoke annonymous function <%p> with args <%p> arity %u",
			(void *)func, (void *)args, signature_count(func->s));
	}
	else
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "Invoke function %s <%p> with args <%p> arity %u",
			func->name, (void *)func, (void *)args, signature_count(func->s));
	}

	for (iterator = 0; iterator < size; ++iterator)
	{
		type t = signature_get_type(func->s, iterator);

		type_id id_arg = value_type_id((value)args[iterator]), id_sig = TYPE_INVALID;

		if (t != NULL)
		{
			id_sig = type_index(t);
		}

		log_write("metacall", LOG_LEVEL_DEBUG, "  #%u Signature Type: %s - Argument Type: %s - Argument Value: %p",
			iterator, type_id_name(id_sig), type_id_name(id_arg), (void *)args[iterator]);

		function_call_value_debug(id_arg, (value)args[iterator]);
	}

	ret = func->interface->invoke(func, func->impl, args, size);

	if (ret != NULL)
	{
		id_ret = value_type_id((value)ret);
	}

	ret_type = signature_get_return(func->s);

	id_ret_sig = value_type_id(ret_type);

	if (func->name == NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "  #ret Signature Type: %s - Argument Type: %s - Argument Value: %p (from annonymous function <%p> with args <%p> arity %u)",
			type_id_name(id_ret_sig), type_id_name(id_ret), (void *)ret, (void *)func, (void *)args, signature_count(func->s));
	}
	else
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "  #ret Signature Type: %s - Argument Type: %s - Argument Value: %p (from function %s <%p> with args <%p> arity %u)",
			type_id_name(id_ret_sig), type_id_name(id_ret), (void *)ret, func->name, (void *)func, (void *)args, signature_count(func->s));
	}

	function_call_value_debug(value_type_id(ret), ret);

	return ret;
}
#endif

function_return function_call(function func, function_args args, size_t size)
{
	if (func == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid function call, function pointer is null");

		return NULL;
	}

	if (args == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid function call, arguments are null");

		return NULL;
	}

	if (func->interface == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid function call, function interface is null");

		return NULL;
	}

	if (func->interface->invoke == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid function call, function interface invoke method is null");

		return NULL;
	}

	/*
	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
		return function_call_debug(func, args, size);
	#else
		return func->interface->invoke(func, func->impl, args, size);
	#endif
	*/

	return func->interface->invoke(func, func->impl, args, size);
}

function_return function_await(function func, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
{
	if (func != NULL && args != NULL)
	{
		if (func->interface != NULL && func->interface->await != NULL)
		{
			if (func->name == NULL)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Await annonymous function with args <%p>", (void *)args);
			}
			else
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Await function (%s) with args <%p>", func->name, (void *)args);
			}

			return func->interface->await(func, func->impl, args, size, resolve_callback, reject_callback, context);
		}
	}

	return NULL;
}

void function_stats_debug()
{
#if !(!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	if (function_stats.allocations != function_stats.deallocations || function_stats.increments != function_stats.decrements)
#endif
	{
		printf("----------------- FUNCTIONS -----------------\n");
		printf("Allocations: %" PRIuS "\n", function_stats.allocations);
		printf("Deallocations: %" PRIuS "\n", function_stats.deallocations);
		printf("Increments: %" PRIuS "\n", function_stats.increments);
		printf("Decrements: %" PRIuS "\n", function_stats.decrements);
		fflush(stdout);
	}
}

void function_destroy(function func)
{
	if (func != NULL)
	{
		if (function_decrement_reference(func) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid reference counter in function: %s", func->name ? func->name : "<anonymous>");
		}

		if (func->ref_count == 0)
		{
			if (func->name == NULL)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Destroy anonymous function <%p>", (void *)func);
			}
			else
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Destroy function %s <%p>", func->name, (void *)func);
			}

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

			++function_stats.deallocations;
		}
	}
}
