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
#include <serial/serial.h>

#include <log/log.h>

#define METACALL "metacall.txt"
#define BUFFER_SIZE 256
#define RAPID_JSON "rapid_json"

// The Curl Libraries
#include "curl/curl.h"
#include "stdio.h"
#include "ctype.h"
#include <stdlib.h>
#include "string.h"

typedef struct loader_impl_rpc_function_type
{
	loader_impl_rpc_handle *handle;
	void *func_rpc_data;

} * loader_impl_rpc_function;

typedef struct loader_impl_rpc_handle_type
{
	void *todo;

} * loader_impl_rpc_handle;

typedef struct loader_impl_rpc_type
{
	CURL *curl;

} * loader_impl_rpc;

typedef struct JsonData
{
	size_t string_length;
	char *data;
	size_t buffer_size;
} JsonData;

void jsondata_constructor(JsonData *this)
{
	this->data = malloc(BUFFER_SIZE);
	this->data[0] = "\0"; // trying to make it an empty null terminated string
	this->string_length = 1;
	this->buffer_size = BUFFER_SIZE;
}

void jsondata_append(JsonData *this, char c)
{
	if (this->string_length < this->buffer_size)
	{
		this->data[this->string_length - 1] = c;
		this->data[this->string_length] = "\0";
		this->string_length++;
	}
	else
	{
		this->data = realloc(this->data, this->buffer_size + BUFFER_SIZE);
		this->buffer_size = this->buffer_size + BUFFER_SIZE;
	}
}

void jsondata_destructor(JsonData *this)
{
	free(this->data);
}

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
			&type_rpc_interface_destroy};

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

function_return function_rpc_interface_await(function func, function_impl impl, function_args args, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)resolve_callback;
	(void)reject_callback;
	(void)context;

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
			&function_rpc_interface_await,
			&function_rpc_interface_destroy};

	return &rpc_function_interface;
}

loader_impl_data rpc_loader_impl_initialize(loader_impl impl, configuration config, loader_host host)
{
	loader_impl_rpc rpc_impl = malloc(sizeof(struct loader_impl_rpc_type));

	(void)impl;
	(void)config;

	if (rpc_impl == NULL)
	{
		return NULL;
	}

	log_copy(host->log);

	curl_global_init(CURL_GLOBAL_ALL);

	rpc_impl->curl = curl_easy_init();

	if (rpc_impl->curl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Could not create CURL object");

		return NULL;
	}

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

loader_handle rpc_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char *buffer, size_t size)
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

static loader_impl_rpc_function rpc_function_create(loader_impl_rpc_handle handle)
{
	loader_impl_rpc_function rpc_func = malloc(sizeof(struct loader_impl_rpc_function_type));
	if (rpc_func != NULL)
	{
		rpc_func->handle = handle;
		rpc_func->func_rpc_data = NULL;
	}

	return rpc_func;
}

static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
}

int rpc_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	FILE *fp = fopen(METACALL, "r");
	if (fp == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Could not find/open api endpoint descriptor file");
		return 1;
	}
	size_t idx = 0, array_idx = 0;
	char currChar;
	char **array_of_strings = malloc(BUFFER_SIZE);
	while ((currChar = fgetc(fp)) != EOF)
	{
		if (idx == BUFFER_SIZE || array_idx == BUFFER_SIZE)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "each endpoint is larger than the buffer size");
			return 1;
		}
		if (isblank(currChar))
			continue;
		char *endpoint = malloc(BUFFER_SIZE); // i believe malloc will always be successfull :).
		if (currChar != "\n")
		{
			endpoint[idx++] = currChar;
		}
		else
		{
			endpoint[idx] = "\0"; // making it a null_terminated string to make it manipulatable by string.h
			idx = 0;			  // resset the counter.
			array_of_strings[array_idx++] = endpoint;
		}
	}

	// making http request
	loader_impl_rpc rpc = loader_impl_get(impl);
	curl_easy_setopt(rpc->curl, CURLOPT_POSTFIELDS, "");
	curl_easy_setopt(rpc->curl, CURLOPT_WRITEFUNCTION, write_data);
	memory_allocator allocator = memory_allocator_std(&malloc, &realloc, &free);
	serial s = serial_create(RAPID_JSON);
	for (size_t i = 0; i < array_idx; i++)
	{

		strcat(array_of_strings[i], "/inspect");
		curl_easy_setopt(rpc->curl, CURLOPT_URL, array_of_strings[i]);
		JsonData jd;
		jsondata_constructor(&jd);
		curl_easy_setopt(rpc->curl, CURLOPT_WRITEDATA, jd);
		CURLcode res = curl_easy_perform(rpc->curl);
		if (res == CURLE_OK)
		{
			
			value map = serial_deserialize(s, jd.data, jd.string_length, allocator);
			value arrayValue = get_funcs(map);
			value *array_of_maps = value_to_array(arrayValue);
			size_t iterator, array_size = value_type_count(arrayValue);
			scope sp = context_scope(ctx);
			for (size_t i = 0; i < array_size; i++)
			{
				loader_impl_rpc_function rpc_func = rpc_function_create((loader_impl_rpc_handle)handle);
				value kmap = array_of_maps[i];
				value *array_of_array = value_to_map(kmap);
				const char *funcName = value_to_string(array_of_array[0][1]);
				kmap = array_of_maps[1][1];
				array_of_array = value_to_map(kmap);
				const char *funcRet = value_to_string(array_of_array[0][1]);
				arrayValue = array_of_array[1][1];
				size_t argc = value_type_count(arrayValue);
				function f = function_create(funcName, argc, rpc_func, &function_rpc_singleton);
				signature s = function_signature(f);
				signature_set_return(s, loader_impl_type(impl, funcRet));
				
			}
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Could not reach Metacall servers");
			return 1;
		}
	}

	// Free all Prisoners(pointers)
}

int rpc_loader_impl_destroy(loader_impl impl)
{
	loader_impl_rpc rpc_impl = loader_impl_get(impl);

	curl_easy_cleanup(rpc_impl->curl);

	curl_global_cleanup();

	return 0;
}

static value get_funcs(value map)
{
	value *map_value = value_to_map(map);
	value *pair = value_to_array(map_value[0]);
	pair = value_to_array(pair[1]); // trying to reuse the variable thats all
	value *map1 = value_to_map(pair[0]);
	pair = value_to_array(map1[1]);
	map1 = value_to_map(pair[1]);
	pair = value_to_array(map1[1]);
	return pair[1]
}