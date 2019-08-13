/*
 *	Loader Library by Parra Studios
 *	A plugin for loading rpc endpoints at run-time into a process.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <rpc_loader/rpc_loader_impl.h>

#include <loader/loader_impl.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

#include <log/log.h>

// The Curl Libraries
#include "curl/curl.h"
#include <stdlib.h>

typedef struct loader_impl_rpc_function_type
{
	void * todo;

} * loader_impl_rpc_function;

typedef struct loader_impl_rpc_handle_type
{
	void * todo;

} * loader_impl_rpc_handle;

typedef struct loader_impl_rpc_type
{
	CURL * curl;

} * loader_impl_rpc;

int type_rpc_interface_create(type t, type_impl impl)
{
	/* TODO */

	(void)t;
	(void)impl;

	return 0;
}

void type_rpc_interface_destroy(type t, type_impl impl)
{
	/* TODO */

	(void)t;
	(void)impl;
}

type_interface type_rpc_singleton(void)
{
	static struct type_interface_type rpc_type_interface =
	{
		&type_rpc_interface_create,
		&type_rpc_interface_destroy
	};

	return &rpc_type_interface;
}

int function_rpc_interface_create(function func, function_impl impl)
{
	/* TODO */

	(void)func;
	(void)impl;

	return 0;
}

function_return function_rpc_interface_invoke(function func, function_impl impl, function_args args)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;

	return NULL;
}

void function_rpc_interface_destroy(function func, function_impl impl)
{
	/* TODO */

	(void)func;
	(void)impl;
}

function_interface function_rpc_singleton(void)
{
	static struct function_interface_type rpc_function_interface =
	{
		&function_rpc_interface_create,
		&function_rpc_interface_invoke,
		&function_rpc_interface_destroy
	};

	return &rpc_function_interface;
}

loader_impl_data rpc_loader_impl_initialize(loader_impl impl, configuration config, loader_host host)
{
	loader_impl_rpc rpc_impl;
	rpc_impl = malloc(sizeof(struct loader_impl_rpc));
	rpc_impl->curl = curl_easy_init()
	if(!rpc_impl->curl){
		log_write("metacall", LOG_LEVEL_ERROR, "Could Not create Curl object");
		return NULL;
	}
	(void)impl;
	(void)config;
	(void)host;

	return rpc_impl;
}

int rpc_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	/* TODO */

	(void)impl;
	(void)path;

	return 0;
}

loader_handle rpc_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	/* TODO */

	(void)impl;
	(void)paths;
	(void)size;

	return (loader_handle)NULL;
}

loader_handle rpc_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size)
{
	/* TODO */

	(void)impl;
	(void)name;
	(void)buffer;
	(void)size;

	return NULL;
}

loader_handle rpc_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	/* TODO */

	(void)impl;
	(void)path;

	return NULL;
}

int rpc_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	/* TODO */

	(void)impl;
	(void)handle;

	return 0;
}

int rpc_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	/* TODO */

	(void)impl;
	(void)handle;
	(void)ctx;

	return 0;
}

int rpc_loader_impl_destroy(loader_impl impl)
{
	loader_impl_rpc rpc_impl = (loader_impl_rpc)impl;
	curl_easy_cleanup(rpc_impl->curl)

	(void)impl;

	return 0;
}
