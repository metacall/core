/*
 *	Loader Library by Parra Studios
 *	A plugin for loading rpc endpoints at run-time into a process.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <portability/portability_path.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>

#include <serial/serial.h>

#include <log/log.h>

#include <metacall/metacall.h>

#include <curl/curl.h>

#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

#include <concurrentqueue.h>

/* Forward declaration for async context */
struct rpc_async_context;

typedef struct loader_impl_rpc_type
{
	CURL *discover_curl;
	CURL *invoke_curl;
	CURLM *async_multi;
	std::thread poll_thread;
	std::atomic<bool> exit_flag;
	moodycamel::ConcurrentQueue<rpc_async_context *> async_queue;
	void *allocator;
	struct curl_slist *headers;
	std::map<type_id, type> types;
	std::set<std::string> execution_paths;

} * loader_impl_rpc;

typedef struct loader_impl_rpc_handle_type
{
	std::vector<std::string> urls;

} * loader_impl_rpc_handle;

typedef struct loader_impl_rpc_function_type
{
	loader_impl_rpc rpc_impl;
	std::string url;

} * loader_impl_rpc_function;

typedef struct loader_impl_rpc_write_data_type
{
	std::string buffer;

} * loader_impl_rpc_write_data;

/* Context for a single async RPC call */
struct rpc_async_context
{
	CURL *easy;
	std::string url;
	loader_impl_rpc_write_data_type write_data;
	function_resolve_callback resolve_callback;
	function_reject_callback reject_callback;
	void *context;
};

static size_t rpc_loader_impl_write_data(void *buffer, size_t size, size_t nmemb, void *userp);
static int rpc_loader_impl_discover_value(loader_impl_rpc rpc_impl, std::string &url, value v, context ctx);
static int rpc_loader_impl_initialize_types(loader_impl impl, loader_impl_rpc rpc_impl);

size_t rpc_loader_impl_write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	loader_impl_rpc_write_data write_data = static_cast<loader_impl_rpc_write_data>(userp);
	const size_t data_len = size * nmemb;
	try
	{
		write_data->buffer.append(static_cast<char *>(buffer), data_len);
	}
	catch (std::bad_alloc &e)
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
	static struct type_interface_type rpc_type_interface = {
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
	loader_impl_rpc_function rpc_function = static_cast<loader_impl_rpc_function>(impl);
	loader_impl_rpc rpc_impl = rpc_function->rpc_impl;
	value v = metacall_value_create_array(NULL, size);
	size_t body_request_size = 0;

	(void)func;

	if (size > 0)
	{
		void **v_array = metacall_value_to_array(v);

		for (size_t arg = 0; arg < size; ++arg)
		{
			v_array[arg] = args[arg];
		}
	}

	char *buffer = metacall_serialize(metacall_serial(), v, &body_request_size, rpc_impl->allocator);

	/* Destroy the value without destroying the contents of the array */
	value_destroy(v);

	if (body_request_size == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serialization of the values to the endpoint %s", rpc_function->url.c_str());
		return NULL;
	}

	/* Execute a POST to the endpoint */
	loader_impl_rpc_write_data_type write_data;

	curl_easy_setopt(rpc_impl->invoke_curl, CURLOPT_URL, rpc_function->url.c_str());
	curl_easy_setopt(rpc_impl->invoke_curl, CURLOPT_POSTFIELDS, buffer);
	curl_easy_setopt(rpc_impl->invoke_curl, CURLOPT_POSTFIELDSIZE, body_request_size - 1);
	curl_easy_setopt(rpc_impl->invoke_curl, CURLOPT_WRITEDATA, static_cast<loader_impl_rpc_write_data>(&write_data));

	CURLcode res = curl_easy_perform(rpc_impl->invoke_curl);

	/* Clear the request buffer */
	metacall_allocator_free(rpc_function->rpc_impl->allocator, buffer);

	if (res != CURLE_OK)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Could not call to the API endpoint %s [%]", rpc_function->url.c_str(), curl_easy_strerror(res));
		return NULL;
	}

	/* Deserialize the call result data */
	const size_t write_data_size = write_data.buffer.length() + 1;

	void *result_value = metacall_deserialize(metacall_serial(), write_data.buffer.c_str(), write_data_size, rpc_impl->allocator);

	if (result_value == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Could not deserialize the call result from API endpoint %s", rpc_function->url.c_str());
	}

	return result_value;
}

/* Poll loop: runs in a single thread, drives all async transfers.
 * Uses lock-free queue (MPSC) + curl_multi_wakeup for zero-mutex design.
 * Only this thread touches the CURLM* multi handle. */
static void rpc_poll_loop(loader_impl_rpc rpc_impl)
{
	int still_running = 0;

	while (true)
	{
		/* Sleep until: network activity OR curl_multi_wakeup OR 1s timeout */
		curl_multi_poll(rpc_impl->async_multi, NULL, 0, 1000, NULL);

		/* Drain queue — add new handles from producers */
		rpc_async_context *ctx;
		while (rpc_impl->async_queue.try_dequeue(ctx))
		{
			curl_multi_add_handle(rpc_impl->async_multi, ctx->easy);
		}

		/* Drive all active transfers forward */
		curl_multi_perform(rpc_impl->async_multi, &still_running);

		/* Check for completed transfers */
		CURLMsg *msg;
		int msgs_left;

		while ((msg = curl_multi_info_read(rpc_impl->async_multi, &msgs_left)))
		{
			if (msg->msg == CURLMSG_DONE)
			{
				CURL *easy = msg->easy_handle;
				CURLcode result = msg->data.result;

				/* Retrieve our async context */
				rpc_async_context *done_ctx = NULL;
				curl_easy_getinfo(easy, CURLINFO_PRIVATE, &done_ctx);

				/* Remove from multi handle */
				curl_multi_remove_handle(rpc_impl->async_multi, easy);
				curl_easy_cleanup(easy);

				if (done_ctx == NULL)
				{
					continue;
				}

				if (result != CURLE_OK)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Async call failed to API endpoint %s [%s]", done_ctx->url.c_str(), curl_easy_strerror(result));

					if (done_ctx->reject_callback != NULL)
					{
						std::string error_msg = std::string("HTTP request failed: ") + curl_easy_strerror(result);
						value error = metacall_value_create_string(error_msg.c_str(), error_msg.length());
						done_ctx->reject_callback(error, done_ctx->context);
						metacall_value_destroy(error);
					}

					delete done_ctx;
					continue;
				}

				/* Deserialize the response */
				const size_t write_data_size = done_ctx->write_data.buffer.length() + 1;

				struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };
				void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

				void *result_value = metacall_deserialize(metacall_serial(), done_ctx->write_data.buffer.c_str(), write_data_size, allocator);

				metacall_allocator_destroy(allocator);

				if (result_value == NULL)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Could not deserialize async call result from API endpoint %s", done_ctx->url.c_str());

					if (done_ctx->reject_callback != NULL)
					{
						value error = metacall_value_create_string("Deserialization failed", sizeof("Deserialization failed") - 1);
						done_ctx->reject_callback(error, done_ctx->context);
						metacall_value_destroy(error);
					}

					delete done_ctx;
					continue;
				}

				/* Call resolve callback with the result */
				if (done_ctx->resolve_callback != NULL)
				{
					done_ctx->resolve_callback(result_value, done_ctx->context);
				}
				else
				{
					metacall_value_destroy(result_value);
				}

				delete done_ctx;
			}
		}

		/* Graceful exit: exit flag + queue drained + no in-flight transfers */
		if (rpc_impl->exit_flag.load() && still_running == 0 && rpc_impl->async_queue.size_approx() == 0)
		{
			break;
		}
	}
}

function_return function_rpc_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
{
	loader_impl_rpc_function rpc_function = static_cast<loader_impl_rpc_function>(impl);
	loader_impl_rpc rpc_impl = rpc_function->rpc_impl;

	(void)func;

	/* Serialize arguments */
	value v = metacall_value_create_array(NULL, size);
	size_t body_request_size = 0;

	if (size > 0)
	{
		void **v_array = metacall_value_to_array(v);

		for (size_t arg = 0; arg < size; ++arg)
		{
			v_array[arg] = args[arg];
		}
	}

	char *buffer = metacall_serialize(metacall_serial(), v, &body_request_size, rpc_impl->allocator);

	value_destroy(v);

	if (body_request_size == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serialization of the values to the endpoint %s", rpc_function->url.c_str());

		if (reject_callback != NULL)
		{
			value error = metacall_value_create_string("Serialization failed", sizeof("Serialization failed") - 1);
			reject_callback(error, context);
			metacall_value_destroy(error);
		}

		return NULL;
	}

	/* Create async context */
	rpc_async_context *async_ctx = new rpc_async_context();
	async_ctx->url = rpc_function->url;
	async_ctx->resolve_callback = resolve_callback;
	async_ctx->reject_callback = reject_callback;
	async_ctx->context = context;

	/* Create easy handle for this async call */
	CURL *easy = curl_easy_init();

	if (easy == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Could not create CURL handle for async call to %s", rpc_function->url.c_str());
		metacall_allocator_free(rpc_impl->allocator, buffer);
		delete async_ctx;
		return NULL;
	}

	curl_easy_setopt(easy, CURLOPT_URL, async_ctx->url.c_str());
	curl_easy_setopt(easy, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(easy, CURLOPT_HEADER, 0L);
	curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(easy, CURLOPT_HTTPHEADER, rpc_impl->headers);
	curl_easy_setopt(easy, CURLOPT_USERAGENT, "librpc_loader/0.1");
	curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, rpc_loader_impl_write_data);
	curl_easy_setopt(easy, CURLOPT_WRITEDATA, static_cast<loader_impl_rpc_write_data>(&async_ctx->write_data));
	curl_easy_setopt(easy, CURLOPT_PRIVATE, async_ctx);

	/* COPYPOSTFIELDS copies data internally, safe to free buffer after */
	curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, (long)(body_request_size - 1));
	curl_easy_setopt(easy, CURLOPT_COPYPOSTFIELDS, buffer);

	async_ctx->easy = easy;

	/* Free serialization buffer */
	metacall_allocator_free(rpc_impl->allocator, buffer);

	/* Enqueue for poll thread — lock-free, wait-free */
	rpc_impl->async_queue.enqueue(async_ctx);

	/* Wake poll thread from curl_multi_poll (thread-safe) */
	curl_multi_wakeup(rpc_impl->async_multi);

	/* Return immediately - result comes via callback */
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
	static struct function_interface_type rpc_function_interface = {
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
		const char *name;
	} type_id_name_pair[] = {
		{ TYPE_BOOL, "Boolean" },
		{ TYPE_CHAR, "Char" },
		{ TYPE_SHORT, "Short" },
		{ TYPE_INT, "Integer" },
		{ TYPE_LONG, "Long" },
		{ TYPE_FLOAT, "Float" },
		{ TYPE_DOUBLE, "Double" },
		{ TYPE_STRING, "String" },
		{ TYPE_BUFFER, "Buffer" },
		{ TYPE_ARRAY, "Array" },
		{ TYPE_MAP, "Map" },
		{ TYPE_PTR, "Ptr" }
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

loader_impl_data rpc_loader_impl_initialize(loader_impl impl, configuration config)
{
	loader_impl_rpc rpc_impl = new loader_impl_rpc_type();

	(void)impl;
	(void)config;

	if (rpc_impl == nullptr)
	{
		return NULL;
	}

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	rpc_impl->allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	if (rpc_impl->allocator == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Could not create allocator for serialization");

		delete rpc_impl;

		return NULL;
	}

	curl_global_init(CURL_GLOBAL_ALL);

	/* Initialize discover CURL object */
	rpc_impl->discover_curl = curl_easy_init();

	if (rpc_impl->discover_curl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Could not create CURL inspect object");

		metacall_allocator_destroy(rpc_impl->allocator);

		delete rpc_impl;

		return NULL;
	}

	curl_easy_setopt(rpc_impl->discover_curl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(rpc_impl->discover_curl, CURLOPT_HEADER, 0L);
	curl_easy_setopt(rpc_impl->discover_curl, CURLOPT_WRITEFUNCTION, rpc_loader_impl_write_data);

	/* Initialize invoke CURL object */
	rpc_impl->invoke_curl = curl_easy_init();

	if (rpc_impl->invoke_curl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Could not create CURL invoke object");

		curl_easy_cleanup(rpc_impl->discover_curl);

		metacall_allocator_destroy(rpc_impl->allocator);

		delete rpc_impl;

		return NULL;
	}

	rpc_impl->headers = NULL;
	rpc_impl->headers = curl_slist_append(rpc_impl->headers, "Accept: application/json");
	rpc_impl->headers = curl_slist_append(rpc_impl->headers, "Content-Type: application/json");
	rpc_impl->headers = curl_slist_append(rpc_impl->headers, "charset: utf-8");

	curl_easy_setopt(rpc_impl->invoke_curl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(rpc_impl->invoke_curl, CURLOPT_HEADER, 0L);
	curl_easy_setopt(rpc_impl->invoke_curl, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(rpc_impl->invoke_curl, CURLOPT_HTTPHEADER, rpc_impl->headers);
	curl_easy_setopt(rpc_impl->invoke_curl, CURLOPT_USERAGENT, "librpc_loader/0.1");
	curl_easy_setopt(rpc_impl->invoke_curl, CURLOPT_WRITEFUNCTION, rpc_loader_impl_write_data);

	/* Initialize async multi handle */
	rpc_impl->async_multi = curl_multi_init();

	if (rpc_impl->async_multi == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Could not create CURL multi handle for async");

		curl_easy_cleanup(rpc_impl->discover_curl);
		curl_easy_cleanup(rpc_impl->invoke_curl);
		metacall_allocator_destroy(rpc_impl->allocator);
		delete rpc_impl;

		return NULL;
	}

	/* Start poll thread for async transfers */
	rpc_impl->exit_flag.store(false);
	rpc_impl->poll_thread = std::thread(rpc_poll_loop, rpc_impl);

	if (rpc_loader_impl_initialize_types(impl, rpc_impl) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Could not create CURL object");

		rpc_impl->exit_flag.store(true);
		curl_multi_wakeup(rpc_impl->async_multi);
		rpc_impl->poll_thread.join();

		curl_multi_cleanup(rpc_impl->async_multi);
		curl_easy_cleanup(rpc_impl->discover_curl);
		curl_easy_cleanup(rpc_impl->invoke_curl);
		metacall_allocator_destroy(rpc_impl->allocator);
		delete rpc_impl;

		return NULL;
	}

	/* Register initialization */
	loader_initialization_register(impl);

	return rpc_impl;
}

int rpc_loader_impl_execution_path(loader_impl impl, const loader_path path)
{
	loader_impl_rpc rpc_impl = static_cast<loader_impl_rpc>(loader_impl_get(impl));

	auto pair = rpc_impl->execution_paths.insert(path);

	return pair.second == true ? 0 : 1;
}

int rpc_loader_impl_load_from_stream_handle(loader_impl_rpc_handle rpc_handle, std::istream &stream)
{
	std::string url;

	while (std::getline(stream, url))
	{
		/* Remove white spaces */
		url.erase(std::remove_if(url.begin(), url.end(), [](char &c) {
			return std::isspace(c);
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

int rpc_loader_impl_load_from_file_handle(loader_impl_rpc_handle rpc_handle, const loader_path path)
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

int rpc_loader_impl_load_from_file_execution_paths(loader_impl_rpc rpc_impl, loader_impl_rpc_handle rpc_handle, const loader_path path)
{
	if (rpc_loader_impl_load_from_file_handle(rpc_handle, path) == 0)
	{
		return 0;
	}

	if (rpc_impl->execution_paths.size() > 0)
	{
		for (auto it : rpc_impl->execution_paths)
		{
			loader_path absolute_path;

			(void)portability_path_join(it.c_str(), it.size(), path, strnlen(path, LOADER_PATH_SIZE) + 1, absolute_path, LOADER_PATH_SIZE);

			if (rpc_loader_impl_load_from_file_handle(rpc_handle, absolute_path) == 0)
			{
				return 0;
			}
		}
	}

	return 1;
}

int rpc_loader_impl_load_from_memory_handle(loader_impl_rpc_handle rpc_handle, const char *buffer, size_t size)
{
	if (size == 0)
	{
		return 1;
	}

	std::string str(buffer, size - 1);
	std::stringstream stream(str);

	return rpc_loader_impl_load_from_stream_handle(rpc_handle, stream);
}

loader_handle rpc_loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size)
{
	loader_impl_rpc_handle rpc_handle = new loader_impl_rpc_handle_type();

	if (rpc_handle == nullptr)
	{
		return NULL;
	}

	loader_impl_rpc rpc_impl = static_cast<loader_impl_rpc>(loader_impl_get(impl));

	for (size_t iterator = 0; iterator < size; ++iterator)
	{
		if (rpc_loader_impl_load_from_file_execution_paths(rpc_impl, rpc_handle, paths[iterator]) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Could not load the URL file descriptor %s", paths[iterator]);

			delete rpc_handle;

			return NULL;
		}
	}

	return static_cast<loader_handle>(rpc_handle);
}

loader_handle rpc_loader_impl_load_from_memory(loader_impl impl, const loader_name name, const char *buffer, size_t size)
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

loader_handle rpc_loader_impl_load_from_package(loader_impl impl, const loader_path path)
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
static std::map<std::string, void *> rpc_loader_impl_value_to_map(void *v)
{
	void **v_map = metacall_value_to_map(v);
	std::map<std::string, void *> m;

	for (size_t iterator = 0; iterator < metacall_value_count(v); ++iterator)
	{
		void **map_pair = metacall_value_to_array(v_map[iterator]);
		const char *key = metacall_value_to_string(map_pair[0]);
		m[key] = map_pair[1];
	}

	return m;
}

int rpc_loader_impl_discover_value(loader_impl_rpc rpc_impl, std::string &url, void *v, context ctx)
{
	void **lang_map = metacall_value_to_map(v);

	for (size_t lang = 0; lang < metacall_value_count(v); ++lang)
	{
		void **lang_pair = metacall_value_to_array(lang_map[lang]);
		void **script_array = metacall_value_to_array(lang_pair[1]);

		for (size_t script = 0; script < metacall_value_count(lang_pair[1]); ++script)
		{
			std::map<std::string, void *> script_map = rpc_loader_impl_value_to_map(script_array[script]);
			std::map<std::string, void *> scope_map = rpc_loader_impl_value_to_map(script_map["scope"]);
			void *funcs = scope_map["funcs"];
			void **funcs_array = metacall_value_to_array(funcs);

			for (size_t func = 0; func < metacall_value_count(funcs); ++func)
			{
				std::map<std::string, void *> func_map = rpc_loader_impl_value_to_map(funcs_array[func]);
				const char *func_name = metacall_value_to_string(func_map["name"]);
				bool is_async = metacall_value_to_bool(func_map["async"]) == 0L ? false : true;
				std::map<std::string, void *> signature_map = rpc_loader_impl_value_to_map(func_map["signature"]);
				void *args = signature_map["args"];
				void **args_array = metacall_value_to_array(args);
				const size_t args_count = metacall_value_count(args);
				loader_impl_rpc_function rpc_func = new loader_impl_rpc_function_type();

				rpc_func->url = url + (is_async ? "await/" : "call/") + func_name;
				rpc_func->rpc_impl = rpc_impl;

				function f = function_create(func_name, args_count, rpc_func, &function_rpc_singleton);

				signature s = function_signature(f);

				function_async(f, is_async == true ? ASYNCHRONOUS : SYNCHRONOUS);

				for (size_t arg = 0; arg < args_count; ++arg)
				{
					std::map<std::string, void *> arg_map = rpc_loader_impl_value_to_map(args_array[arg]);
					std::map<std::string, void *> type_map = rpc_loader_impl_value_to_map(arg_map["type"]);
					void *id_v = metacall_value_copy(type_map["id"]);
					type_id id = metacall_value_cast_int(&id_v);

					metacall_value_destroy(id_v);

					signature_set(s, arg, metacall_value_to_string(arg_map["name"]), rpc_impl->types[id]);
				}

				std::map<std::string, void *> ret_map = rpc_loader_impl_value_to_map(signature_map["ret"]);
				std::map<std::string, void *> type_map = rpc_loader_impl_value_to_map(ret_map["type"]);
				void *id_v = metacall_value_copy(type_map["id"]);
				type_id id = metacall_value_cast_int(&id_v);

				metacall_value_destroy(id_v);

				signature_set_return(s, rpc_impl->types[id]);

				scope sp = context_scope(ctx);
				value v = value_create_function(f);

				if (scope_define(sp, function_name(f), v) != 0)
				{
					metacall_value_destroy(v);
					return 1;
				}
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

		curl_easy_setopt(rpc_impl->discover_curl, CURLOPT_URL, inspect_url.c_str());
		curl_easy_setopt(rpc_impl->discover_curl, CURLOPT_WRITEDATA, static_cast<loader_impl_rpc_write_data>(&write_data));

		CURLcode res = curl_easy_perform(rpc_impl->discover_curl);

		if (res != CURLE_OK)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Could not access the API endpoint %s [%s]", rpc_handle->urls[iterator].c_str(), curl_easy_strerror(res));
			return 1;
		}

		/* Deserialize the inspect data */
		const size_t size = write_data.buffer.length() + 1;

		void *inspect_value = metacall_deserialize(metacall_serial(), write_data.buffer.c_str(), size, rpc_impl->allocator);

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
	loader_impl_rpc rpc_impl = static_cast<loader_impl_rpc>(loader_impl_get(impl));

	/* Destroy children loaders */
	loader_unload_children(impl);

	/* Stop the poll thread — set exit flag, wake it, wait for drain */
	rpc_impl->exit_flag.store(true);
	curl_multi_wakeup(rpc_impl->async_multi);
	if (rpc_impl->poll_thread.joinable())
	{
		rpc_impl->poll_thread.join();
	}

	/* Clean up async multi handle */
	curl_multi_cleanup(rpc_impl->async_multi);

	curl_slist_free_all(rpc_impl->headers);

	metacall_allocator_destroy(rpc_impl->allocator);

	curl_easy_cleanup(rpc_impl->discover_curl);

	curl_easy_cleanup(rpc_impl->invoke_curl);

	curl_global_cleanup();

	delete rpc_impl;

	return 0;
}
