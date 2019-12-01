/*
 *	Loader Library by Parra Studios
 *	A plugin for loading rpc endpoints at run-time into a process.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use thiz file except in compliance with the License.
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
#include <metacall/metacall.h>
#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>
#include <memory/memory_allocator_std.h>
//#include <serial/serial.h>

#include <log/log.h>

#define METACALL "metacall.txt"
#define BUFFER_SIZE 256
#define RAPID_JSON "rapid_json"

// The Curl Libraries
#include "curl/curl.h"

// standard libraries
#include "functional"
#include "stdio.h"
#include "ctype.h"
#include <stdlib.h>
#include "string.h"
#include "vector"

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
	std::string url;
	std::string response;
} JsonData;

void jsondata_constructor(JsonData *thiz, std::string url)
{
	thiz->url = url;
}

void jsondata_append(JsonData *thiz, char c)
{
	
}

void jsondata_destructor(JsonData *thiz)
{

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
	loader_impl_rpc rpc_impl = (loader_impl_rpc)malloc(sizeof(struct loader_impl_rpc_type));

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
	loader_impl_rpc_function rpc_func = (loader_impl_rpc_function)malloc(sizeof(struct loader_impl_rpc_function_type));
	if (rpc_func != NULL)
	{
		rpc_func->handle = (loader_impl_rpc_handle *)handle;
		rpc_func->func_rpc_data = NULL;
	}

	return rpc_func;
}

static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	JsonData *jd = (JsonData*)userp;
	char *buf = static_cast<char *>(buffer);
	for (size_t i = 0; i < realsize; i++)
	{
		jd->response.append(&buf[i]);
	}
	
}

int rpc_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_rpc rpc_impl = (loader_impl_rpc)loader_impl_get(impl);
	FILE *fp = fopen(METACALL, "r");
	if (fp == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Could not find/open api endpoint descriptor file");
		return 1;
	}
	std::vector<std::string> vctrSrings(15);
	char currChar;
	std::string* endpoint = new std::string();
	while ((currChar = fgetc(fp)) != EOF)
	{

		if (isblank(currChar))
			continue;
		else if (currChar == '\n')
		{
			if(endpoint->length() == 0){
				continue;
			} else if(endpoint->length() > 0){
				vctrSrings.push_back(*endpoint);
				endpoint->clear();
			}
		}
		else
		{
			endpoint->append(&currChar);
		}
	}

	memory_allocator allocator = memory_allocator_std(&malloc, &realloc, &free);
	JsonData jd;

	for (auto &&i : vctrSrings)
	{
		jsondata_constructor(&jd, i);
		i.append("/inspect");
		curl_easy_setopt(rpc_impl->curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(rpc_impl->curl, CURLOPT_WRITEDATA, &jd);
		curl_easy_setopt(rpc_impl->curl, CURLOPT_URL, i);
		CURLcode res = curl_easy_perform(rpc_impl->curl);
		if(res != CURLE_OK && jd.response.length() == 0) {
			// Do some logging here
			continue;
		}
		command_inspect(jd.response.c_str(), jd.response.length(), allocator, [](const std::string, size_t, void *){
			// TODO get params and create objects
		});
		
	}
	// TODO handle inpect funtion output
	// Free all Prisoners(pointers)0.0
}

int rpc_loader_impl_destroy(loader_impl impl)
{
	loader_impl_rpc rpc_impl = (loader_impl_rpc)loader_impl_get(impl);

	curl_easy_cleanup(rpc_impl->curl);

	curl_global_cleanup();

	return 0;
}

void value_array_for_each(void *v, const std::function<void(void *)> &lambda)
{
	void **v_array = static_cast<void **>(metacall_value_to_array(v));
	size_t count = metacall_value_count(v);

	std::for_each(v_array, v_array + count, lambda);
}

void value_map_for_each(void *v, const std::function<void(const char *, void *)> &lambda)
{
	void **v_map = static_cast<void **>(metacall_value_to_map(v));
	size_t count = metacall_value_count(v);

	std::for_each(v_map, v_map + count, [&lambda](void *element) {
		void **v_element = metacall_value_to_array(element);
		lambda(metacall_value_to_string(v_element[0]), v_element[1]);
	});
}

static void command_inspect(const char *str, size_t size, memory_allocator allocator, const std::function<void(const std::string, size_t, void *)> &functionLambda)
{
	void *v = metacall_deserialize(metacall_serial(), str, size, allocator);

	if (v == NULL)
	{
		// TODO(std::cout << "Invalid deserialization" << std::endl);
		return;
	}

	value_map_for_each(v, [](const char *key, void *modules) {
		if (metacall_value_count(modules) == 0)
		{
			return;
		}

		value_array_for_each(modules, [](void *module) {
			/* Get module name */
			void **v_module_map = static_cast<void **>(metacall_value_to_map(module));
			void **v_module_name_tuple = metacall_value_to_array(v_module_map[0]);
			const char *name = metacall_value_to_string(v_module_name_tuple[1]);

			/* Get module functions */
			void **v_module_scope_tuple = metacall_value_to_array(v_module_map[1]);
			void **v_scope_map = metacall_value_to_map(v_module_scope_tuple[1]);
			void **v_scope_funcs_tuple = metacall_value_to_array(v_scope_map[1]);

			if (metacall_value_count(v_scope_funcs_tuple[1]) != 0)
			{
				value_array_for_each(v_scope_funcs_tuple[1], [](void *func) {
					/* Get function name */
					void **v_func_map = static_cast<void **>(metacall_value_to_map(func));
					void **v_func_tuple = metacall_value_to_array(v_func_map[0]);
					const char *func_name = metacall_value_to_string(v_func_tuple[1]);

					/* Get function signature */
					void **v_signature_tuple = metacall_value_to_array(v_func_map[1]);
					void **v_args_map = metacall_value_to_map(v_signature_tuple[1]);
					void **v_args_tuple = metacall_value_to_array(v_args_map[1]);
					void **v_args_array = metacall_value_to_array(v_args_tuple[1]);

					size_t iterator = 0, count = metacall_value_count(v_args_tuple[1]);

					value_array_for_each(v_args_array, [&iterator, &count](void *arg) {
						void **v_arg_map = metacall_value_to_map(arg);
						void **v_arg_name_tupla = metacall_value_to_array(v_arg_map[0]);

						//TODO call parent lambda and pass function params

						++iterator;
					});
				});
			}
		});
	});
}