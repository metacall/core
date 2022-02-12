/*
 *	Loader Library by Parra Studios
 *	A library for loading executable code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

/* -- Headers -- */

#include <loader/loader_host.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_scope.h>

/* -- Definitions -- */

#define LOADER_HOST_NAME "__metacall_host__"

portability_static_assert((size_t)sizeof(LOADER_HOST_NAME) <= (size_t)LOADER_TAG_SIZE,
	"Loader host tag size exceeds the size limit of LOADER_TAG_SIZE");

/* -- Member Data -- */

struct loader_host_invoke_type
{
	loader_register_invoke invoke;
};

/* -- Type Declarations -- */

typedef struct loader_host_invoke_type *loader_host_invoke;

/* -- Private Methods -- */

static value function_host_interface_invoke(function func, function_impl func_impl, function_args args, size_t size);

static function_return function_host_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context);

static void function_host_interface_destroy(function func, function_impl func_impl);

static function_interface function_host_singleton(void);

static void loader_host_destroy_dtor(plugin p);

static void loader_host_destroy(loader_impl host);

/* -- Methods -- */

function_return function_host_interface_invoke(function func, function_impl func_impl, function_args args, size_t size)
{
	loader_host_invoke host_invoke = (loader_host_invoke)func_impl;

	void *data = function_closure(func);

	return host_invoke->invoke(size, args, data);
}

function_return function_host_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
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

void function_host_interface_destroy(function func, function_impl func_impl)
{
	(void)func;

	if (func_impl != NULL)
	{
		free(func_impl);
	}
}

function_interface function_host_singleton(void)
{
	static struct function_interface_type host_interface = {
		NULL,
		&function_host_interface_invoke,
		&function_host_interface_await,
		&function_host_interface_destroy
	};

	return &host_interface;
}

void loader_host_destroy_dtor(plugin p)
{
	loader_impl host = plugin_impl_type(p, loader_impl);

	if (host != NULL)
	{
		loader_host_destroy(host);
	}
}

void loader_host_destroy(loader_impl host)
{
	loader_impl_destroy_objects(host);
	loader_impl_destroy_deallocate(host);
}

plugin loader_host_initialize(void)
{
	loader_impl host = loader_impl_create_host(LOADER_HOST_NAME);
	type_id id;

	if (host == NULL)
	{
		return NULL;
	}

	for (id = 0; id < TYPE_SIZE; ++id)
	{
		type t = type_create(id, type_id_name(id), NULL, NULL);

		if (t != NULL)
		{
			if (loader_impl_type_define(host, type_name(t), t) != 0)
			{
				type_destroy(t);
				goto error;
			}
		}
	}

	plugin p = plugin_create(LOADER_HOST_NAME, NULL, NULL, host, &loader_host_destroy_dtor);

	if (p == NULL)
	{
		goto error;
	}

	return p;
error:
	loader_host_destroy(host);
	return NULL;
}

int loader_host_register(loader_impl host, const char *name, loader_register_invoke invoke, function *func, type_id return_type, size_t arg_size, type_id args_type_id[])
{
	loader_host_invoke host_invoke = malloc(sizeof(struct loader_host_invoke_type));

	if (host_invoke == NULL)
	{
		return 1;
	}

	host_invoke->invoke = invoke;

	function f = function_create(name, arg_size, host_invoke, &function_host_singleton);

	if (f == NULL)
	{
		free(host_invoke);
		return 1;
	}

	signature s = function_signature(f);

	if (arg_size > 0)
	{
		size_t iterator;

		for (iterator = 0; iterator < arg_size; ++iterator)
		{
			static const char empty_argument_name[] = "";

			type t = loader_impl_type(host, type_id_name(args_type_id[iterator]));

			signature_set(s, iterator, empty_argument_name, t);
		}
	}

	type t = loader_impl_type(host, type_id_name(return_type));

	signature_set_return(s, t);

	if (name != NULL)
	{
		context ctx = loader_impl_context(host);
		scope sp = context_scope(ctx);
		scope_define(sp, name, value_create_function(f));
	}

	if (func != NULL)
	{
		*func = f;
	}

	return 0;
}
