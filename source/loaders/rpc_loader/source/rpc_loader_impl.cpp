/*
 *	Loader Library by Parra Studios
 *	A plugin for loading rpc endpoints at run-time into a process.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>
#include <serial/serial.h>
#include <metacall/metacall.h>

#include <log/log.h>

#include <curl/curl.h>

#include <stdlib.h>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <algorithm>
#include <sstream>

typedef struct loader_impl_rpc_write_data_type
{
	std::string buffer;

} * loader_impl_rpc_write_data;

typedef struct loader_impl_rpc_function_type
{
	std::string url;

} * loader_impl_rpc_function;

typedef struct loader_impl_rpc_handle_type
{
	std::vector<std::string> urls;

} * loader_impl_rpc_handle;

typedef struct loader_impl_rpc_type
{
	CURL * curl;
	void * allocator;
	std::map<type_id, type> types;

} * loader_impl_rpc;

static size_t rpc_loader_impl_write_data(void * buffer, size_t size, size_t nmemb, void * userp);
static int rpc_loader_impl_discover_value(loader_impl_rpc rpc_impl, std::string & url, value v, context ctx);
static int rpc_loader_impl_initialize_types(loader_impl impl, loader_impl_rpc rpc_impl);

size_t rpc_loader_impl_write_data(void * buffer, size_t size, size_t nmemb, void * userp)
{
	loader_impl_rpc_write_data write_data = static_cast<loader_impl_rpc_write_data>(userp);
	const size_t data_len = size * nmemb;
	try
	{
		write_data->buffer.append(static_cast<char *>(buffer), data_len);
	}
	catch(std::bad_alloc &e)
	{
		return 0;
	}
	return data_len;
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

function_return function_rpc_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)size;

	return NULL;
}

function_return function_rpc_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
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

void function_rpc_interface_destroy(function func, function_impl impl)
{
	loader_impl_rpc_function rpc_func = static_cast<loader_impl_rpc_function>(impl);

	(void)func;

	delete rpc_func;
}

function_interface function_rpc_singleton(void)
{
	static struct function_interface_type rpc_function_interface =
	{
		&function_rpc_interface_create,
		&function_rpc_interface_invoke,
		&function_rpc_interface_await,
		&function_rpc_interface_destroy
	};

	return &rpc_function_interface;
}

int rpc_loader_impl_initialize_types(loader_impl impl, loader_impl_rpc rpc_impl)
{
	/* TODO: move this to loader_impl by passing the structure and loader_impl_derived callback */

	static struct
	{
		type_id id;
		const char * name;
	}
	type_id_name_pair[] =
	{
		{ TYPE_BOOL,	"Boolean"	},
		{ TYPE_CHAR,	"Char"		},
		{ TYPE_SHORT,	"Short"		},
		{ TYPE_INT,		"Integer"	},
		{ TYPE_LONG,	"Long"		},
		{ TYPE_FLOAT,	"Float"		},
		{ TYPE_DOUBLE,	"Double"	},
		{ TYPE_STRING,	"String"	},
		{ TYPE_BUFFER,	"Buffer"	},
		{ TYPE_ARRAY,	"Array"		},
		{ TYPE_MAP,		"Map"		},
		{ TYPE_PTR,		"Ptr"		}
	};

	size_t index, size = sizeof(type_id_name_pair) / sizeof(type_id_name_pair[0]);

	for (index = 0; index < size; ++index)
	{
		type t = type_create(type_id_name_pair[index].id, type_id_name_pair[index].name, NULL, &type_rpc_singleton);

		if (t != NULL)
		{
			if (loader_impl_type_define(impl, type_name(t), t) != 0)
			{
				type_destroy(t);

				return 1;
			}

			rpc_impl->types[type_id_name_pair[index].id] = t;
		}
	}

	return 0;
}

loader_impl_data rpc_loader_impl_initialize(loader_impl impl, configuration config, loader_host host)
{
	loader_impl_rpc rpc_impl = new loader_impl_rpc_type();

	(void)impl;
	(void)config;

	if (rpc_impl == nullptr)
	{
		return NULL;
	}

	loader_copy(host);

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	rpc_impl->allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	if (rpc_impl->allocator == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Could not create allocator for serialization");

		delete rpc_impl;

		return NULL;
	}

	curl_global_init(CURL_GLOBAL_ALL);

	rpc_impl->curl = curl_easy_init();

	if (!(rpc_impl->curl != NULL && rpc_loader_impl_initialize_types(impl, rpc_impl) == 0))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Could not create CURL object");

		metacall_allocator_destroy(rpc_impl->allocator);

		delete rpc_impl;

		return NULL;
	}

	/* Set up curl general options */
    curl_easy_setopt(rpc_impl->curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(rpc_impl->curl, CURLOPT_HEADER, 0L);

	/* Register initialization */
	loader_initialization_register(impl);

	return rpc_impl;
}

int rpc_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	/* TODO */

	(void)impl;
	(void)path;

	return 0;
}

int rpc_loader_impl_load_from_stream_handle(loader_impl_rpc_handle rpc_handle, std::istream & stream)
{
	std::string url;

	while (std::getline(stream, url))
	{
		/* Remove white spaces */
		url.erase(std::remove_if(url.begin(), url.end(), [](char & c) {
			return std::isspace<char>(c, std::locale::classic());
		}),
		url.end());

		/* Skip empty lines */
		if (url.length() == 0)
		{
			continue;
		}

		/* URL must come without URL encoded parameters */
		if (url[url.length() - 1] != '/')
		{
			url.append("/");
		}

		rpc_handle->urls.push_back(url);
	}

	return 0;
}

int rpc_loader_impl_load_from_file_handle(loader_impl_rpc_handle rpc_handle, const loader_naming_path path)
{
	std::fstream file;

	file.open(path, std::ios::in);

	if (!file.is_open())
	{
		return 1;
	}

	int result = rpc_loader_impl_load_from_stream_handle(rpc_handle, file);

	file.close();

	return result;
}

int rpc_loader_impl_load_from_memory_handle(loader_impl_rpc_handle rpc_handle, const char * buffer, size_t size)
{
	if (size == 0)
	{
		return 1;
	}

	std::string str(buffer, size - 1);
	std::stringstream stream(str);

	return rpc_loader_impl_load_from_stream_handle(rpc_handle, stream);
}

loader_handle rpc_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	loader_impl_rpc_handle rpc_handle = new loader_impl_rpc_handle_type();

	(void)impl;

	if (rpc_handle == nullptr)
	{
		return NULL;
	}

	for (size_t iterator = 0; iterator < size; ++iterator)
	{
		if (rpc_loader_impl_load_from_file_handle(rpc_handle, paths[iterator]) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Could not load the URL file descriptor %s", paths[iterator]);

			delete rpc_handle;

			return NULL;
		}
	}

	return static_cast<loader_handle>(rpc_handle);
}

loader_handle rpc_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size)
{
	loader_impl_rpc_handle rpc_handle = new loader_impl_rpc_handle_type();

	(void)impl;
	(void)name;

	if (rpc_handle == nullptr)
	{
		return NULL;
	}

	if (rpc_loader_impl_load_from_memory_handle(rpc_handle, buffer, size) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Could not load the URL file descriptor %s", buffer);

		delete rpc_handle;

		return NULL;
	}

	return static_cast<loader_handle>(rpc_handle);
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
	loader_impl_rpc_handle rpc_handle = static_cast<loader_impl_rpc_handle>(handle);

	(void)impl;

	rpc_handle->urls.clear();

	delete rpc_handle;

	return 0;
}

// TODO: Move this to the C++ Port
static std::map<std::string, void *> rpc_loader_impl_value_to_map(void * v)
{
	void ** v_map = metacall_value_to_map(v);
	std::map<std::string, void *> m;

	for (size_t iterator = 0; iterator < metacall_value_count(v); ++iterator)
	{
		void ** map_pair = metacall_value_to_array(v_map[iterator]);
		const char * key = metacall_value_to_string(map_pair[0]);
		m[key] = map_pair[1];
	}

	return m;
}

int rpc_loader_impl_discover_value(loader_impl_rpc rpc_impl, std::string & url, void * v, context ctx)
{
	void ** lang_map = metacall_value_to_map(v);

	for (size_t lang = 0; lang < metacall_value_count(v); ++lang)
	{
		void ** lang_pair = metacall_value_to_array(lang_map[lang]);
		void ** script_array = metacall_value_to_array(lang_pair[1]);

		for (size_t script = 0; script < metacall_value_count(lang_pair[1]); ++script)
		{
			std::map<std::string, void *> script_map = rpc_loader_impl_value_to_map(script_array[script]);
			std::map<std::string, void *> scope_map = rpc_loader_impl_value_to_map(script_map["scope"]);
			void * funcs = scope_map["funcs"];
			void ** funcs_array = metacall_value_to_array(funcs);

			for (size_t func = 0; func < metacall_value_count(funcs); ++func)
			{
				std::map<std::string, void *> func_map = rpc_loader_impl_value_to_map(funcs_array[func]);
				const char * func_name = metacall_value_to_string(func_map["name"]);
				bool is_async = metacall_value_to_bool(func_map["async"]) == 0L ? false : true;
				std::map<std::string, void *> signature_map = rpc_loader_impl_value_to_map(func_map["signature"]);
				void * args = signature_map["args"];
				void ** args_array = metacall_value_to_array(args);
				const size_t args_count = metacall_value_count(args);
				loader_impl_rpc_function rpc_func = new loader_impl_rpc_function_type();

				rpc_func->url = url + (is_async ? "await/" : "call/") + func_name;

				function f = function_create(func_name, args_count, rpc_func, &function_rpc_singleton);

				signature s = function_signature(f);

				function_async(f, is_async == true ? FUNCTION_ASYNC : FUNCTION_SYNC);

				for (size_t arg = 0; arg < args_count; ++arg)
				{
					std::map<std::string, void *> arg_map = rpc_loader_impl_value_to_map(args_array[arg]);
					std::map<std::string, void *> type_map = rpc_loader_impl_value_to_map(arg_map["type"]);
					void * id_v = metacall_value_copy(type_map["id"]);
					type_id id = metacall_value_cast_int(&id_v);

					metacall_value_destroy(id_v);

					signature_set(s, arg, metacall_value_to_string(arg_map["name"]), rpc_impl->types[id]);
				}

				std::map<std::string, void *> ret_map = rpc_loader_impl_value_to_map(signature_map["ret"]);
				std::map<std::string, void *> type_map = rpc_loader_impl_value_to_map(ret_map["type"]);
				void * id_v = metacall_value_copy(type_map["id"]);
				type_id id = metacall_value_cast_int(&id_v);

				metacall_value_destroy(id_v);

				signature_set_return(s, rpc_impl->types[id]);

				scope sp = context_scope(ctx);

				scope_define(sp, function_name(f), value_create_function(f));
			}
		}
	}

	metacall_value_destroy(v);

	return 0;
}

int rpc_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_rpc rpc_impl = static_cast<loader_impl_rpc>(loader_impl_get(impl));
	loader_impl_rpc_handle rpc_handle = static_cast<loader_impl_rpc_handle>(handle);

	for (size_t iterator = 0; iterator < rpc_handle->urls.size(); ++iterator)
	{
		loader_impl_rpc_write_data_type write_data;

		std::string inspect_url = rpc_handle->urls[iterator] + "inspect";

		curl_easy_setopt(rpc_impl->curl, CURLOPT_URL, inspect_url.c_str());
		curl_easy_setopt(rpc_impl->curl, CURLOPT_WRITEFUNCTION, rpc_loader_impl_write_data);
		curl_easy_setopt(rpc_impl->curl, CURLOPT_WRITEDATA, static_cast<loader_impl_rpc_write_data>(&write_data));

		CURLcode res = curl_easy_perform(rpc_impl->curl);

		if (res != CURLE_OK)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Could not access the API endpoint %s", rpc_handle->urls[iterator].c_str());
			return 1;
		}

		/* Deserialize the inspect data */
		const size_t size = write_data.buffer.length() + 1;

		void * inspect_value = metacall_deserialize(metacall_serial(), write_data.buffer.c_str(), size, rpc_impl->allocator);

		if (inspect_value == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Could not deserialize the inspect data from API endpoint %s", rpc_handle->urls[iterator].c_str());
			return 1;
		}

		/* Discover the functions from the inspect value */
		if (rpc_loader_impl_discover_value(rpc_impl, rpc_handle->urls[iterator], inspect_value, ctx) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid inspect value discover from API endpoint %s", rpc_handle->urls[iterator].c_str());
			return 1;
		}
	}

	return 0;
}

int rpc_loader_impl_destroy(loader_impl impl)
{
	loader_impl_rpc rpc_impl = (loader_impl_rpc)loader_impl_get(impl);

	/* Destroy children loaders */
	loader_unload_children();

	metacall_allocator_destroy(rpc_impl->allocator);

	curl_easy_cleanup(rpc_impl->curl);

	curl_global_cleanup();

	delete rpc_impl;

	return 0;
}
