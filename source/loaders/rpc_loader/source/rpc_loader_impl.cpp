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

#include <concurrentqueue.h>

#include <metacall/metacall.hpp>

#include <curl/curl.h>

#include <cstdlib>
#include <cstring>

#include <atomic>
#include <chrono>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <thread>
#include <vector>

#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	#define CURL_VERBOSE 1L
#else
	#define CURL_VERBOSE 0L
#endif

/* Forward declaration for async context */
struct rpc_async_context;

typedef struct loader_impl_rpc_type
{
	CURL *discover_curl;
	CURLM *async_multi;
	std::thread poll_thread;
	std::atomic<bool> exit_flag;
	moodycamel::ConcurrentQueue<rpc_async_context *> async_queue;
	void *allocator;
	metacall::detail::serial cached_serial;
	struct curl_slist *headers;
	std::map<type_id, type> types;
	std::set<std::string> execution_paths;
	long timeout_ms;
	int retry_count;
	long retry_delay_ms;

} * loader_impl_rpc;

typedef struct loader_impl_rpc_handle_type
{
	std::vector<metacall::map_typed<std::string, metacall::value>> configs;

} * loader_impl_rpc_handle;

typedef struct loader_impl_rpc_function_type
{
	loader_impl_rpc_function_type(loader_impl_rpc rpc_impl, const std::string &url, bool is_async, const std::string &func_name) :
		rpc_impl(rpc_impl), url(url + (is_async ? "await/" : "call/") + func_name), timeout_ms(rpc_impl->timeout_ms) {}

	loader_impl_rpc rpc_impl;
	std::string url;
	long timeout_ms;

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
static int rpc_loader_impl_discover_value(loader_impl_rpc rpc_impl, const std::string &url, value v, context ctx);
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
	value v = metacall::metacall_value_create_array(NULL, size);
	size_t body_request_size = 0;

	(void)func;

	if (size > 0)
	{
		void **v_array = metacall::metacall_value_to_array(v);

		for (size_t arg = 0; arg < size; ++arg)
		{
			v_array[arg] = args[arg];
		}
	}

	char *buffer = serial_serialize(rpc_impl->cached_serial, (value)v, &body_request_size, (metacall::detail::memory_allocator)rpc_impl->allocator);

	/* Destroy the value without destroying the contents of the array */
	value_destroy(v);

	if (body_request_size == 0)
	{
		log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Invalid serialization of the values to the endpoint %s", rpc_function->url.c_str());
		return NULL;
	}

	/* This creates a per-call CURL handle for thread safety */
	CURL *easy = curl_easy_init();

	if (easy == NULL)
	{
		log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Could not create CURL handle for sync call to %s", rpc_function->url.c_str());
		metacall::metacall_allocator_free(rpc_impl->allocator, buffer);
		return NULL;
	}

	loader_impl_rpc_write_data_type write_data;

	curl_easy_setopt(easy, CURLOPT_VERBOSE, CURL_VERBOSE);
	curl_easy_setopt(easy, CURLOPT_HEADER, 0L);
	curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(easy, CURLOPT_HTTPHEADER, rpc_impl->headers);
	curl_easy_setopt(easy, CURLOPT_USERAGENT, "librpc_loader/0.1");
	curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, rpc_loader_impl_write_data);
	curl_easy_setopt(easy, CURLOPT_URL, rpc_function->url.c_str());
	curl_easy_setopt(easy, CURLOPT_POSTFIELDS, buffer);
	curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, body_request_size - 1);
	curl_easy_setopt(easy, CURLOPT_WRITEDATA, static_cast<loader_impl_rpc_write_data>(&write_data));
	curl_easy_setopt(easy, CURLOPT_TIMEOUT_MS, rpc_function->timeout_ms);

	CURLcode res;

	/* Skip curl_multi_perform and curl_multi_cleanup use-of-uninitialized-value from heap allocation in uninstrumented libcurl */
	memory_sanitizer_uninstrumented({
		res = curl_easy_perform(easy);
		curl_easy_cleanup(easy);
	});

	/* Clear the request buffer */
	metacall::metacall_allocator_free(rpc_function->rpc_impl->allocator, buffer);

	if (res != CURLE_OK)
	{
		log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Could not call to the API endpoint %s [%s]", rpc_function->url.c_str(), curl_easy_strerror(res));
		return NULL;
	}

	/* Deserialize the call result data */
	const size_t write_data_size = write_data.buffer.length() + 1;

	void *result_value = serial_deserialize(rpc_impl->cached_serial, write_data.buffer.c_str(), write_data_size, (metacall::detail::memory_allocator)rpc_impl->allocator);

	if (result_value == NULL)
	{
		log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Could not deserialize the call result from API endpoint %s", rpc_function->url.c_str());
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

		/* Drain queue, add new handles from producers */
		rpc_async_context *ctx;
		while (rpc_impl->async_queue.try_dequeue(ctx))
		{
			curl_multi_add_handle(rpc_impl->async_multi, ctx->easy);
		}

		/* Skip curl_multi_perform use-of-uninitialized-value from heap allocation of curl_mvaprintf in uninstrumented libcurl */
		memory_sanitizer_uninstrumented({
			/* Drive all active transfers forward */
			curl_multi_perform(rpc_impl->async_multi, &still_running);
		});

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

				/* Skip curl_easy_cleanup and curl_multi_cleanup use-of-uninitialized-value from heap allocation in uninstrumented libcurl */
				memory_sanitizer_uninstrumented({
					curl_easy_cleanup(easy);
				});

				if (done_ctx == NULL)
				{
					continue;
				}

				if (result != CURLE_OK)
				{
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
					log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Async HTTP request failed [%s]: %s", done_ctx->url.c_str(), curl_easy_strerror(result));
#endif

					if (done_ctx->reject_callback != NULL)
					{
						value error = metacall::metacall_error_throw("RPCLoader", 0, "", "Async HTTP request failed [%s]: %s", done_ctx->url.c_str(), curl_easy_strerror(result));
						done_ctx->reject_callback(error, done_ctx->context);
						metacall::metacall_value_destroy(error);
					}

					delete done_ctx;
					continue;
				}

				/* Deserialize the response */
				const size_t write_data_size = done_ctx->write_data.buffer.length() + 1;

				struct metacall::metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };
				void *allocator = metacall_allocator_create(metacall::METACALL_ALLOCATOR_STD, (void *)&std_ctx);

				void *result_value = serial_deserialize(rpc_impl->cached_serial, done_ctx->write_data.buffer.c_str(), write_data_size, (metacall::detail::memory_allocator)allocator);

				metacall::metacall_allocator_destroy(allocator);

				if (result_value == NULL)
				{
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
					log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Could not deserialize async call result from API endpoint %s", done_ctx->url.c_str());
#endif

					if (done_ctx->reject_callback != NULL)
					{
						value error = metacall::metacall_error_throw("RPCLoader", 0, "", "Async HTTP deserialization failed [%s]", done_ctx->url.c_str());
						done_ctx->reject_callback(error, done_ctx->context);
						metacall::metacall_value_destroy(error);
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
					metacall::metacall_value_destroy(result_value);
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
	value v = metacall::metacall_value_create_array(NULL, size);
	size_t body_request_size = 0;

	if (size > 0)
	{
		void **v_array = metacall::metacall_value_to_array(v);

		for (size_t arg = 0; arg < size; ++arg)
		{
			v_array[arg] = args[arg];
		}
	}

	char *buffer = serial_serialize(rpc_impl->cached_serial, (value)v, &body_request_size, (metacall::detail::memory_allocator)rpc_impl->allocator);

	/* Destroy the value without destroying the contents of the array */
	value_destroy(v);

	if (body_request_size == 0)
	{
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
		log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Invalid serialization of the values to the endpoint %s", rpc_function->url.c_str());
#endif

		// TODO: Is the reject correct here? Should not we return the exception instead? The function has not been invoked yet
		if (reject_callback != NULL)
		{
			value error = metacall::metacall_error_throw("RPCLoader", 0, "", "Async HTTP serialization failed [%s]", rpc_function->url.c_str());
			reject_callback(error, context);
			metacall::metacall_value_destroy(error);
		}

		return NULL; // TODO: Return here the thrown exception?
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
		log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Could not create CURL handle for async call to %s", rpc_function->url.c_str());
		metacall::metacall_allocator_free(rpc_impl->allocator, buffer);
		delete async_ctx;
		return NULL;
	}

	curl_easy_setopt(easy, CURLOPT_URL, async_ctx->url.c_str());
	curl_easy_setopt(easy, CURLOPT_VERBOSE, CURL_VERBOSE);
	curl_easy_setopt(easy, CURLOPT_HEADER, 0L);
	curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(easy, CURLOPT_HTTPHEADER, rpc_impl->headers);
	curl_easy_setopt(easy, CURLOPT_USERAGENT, "librpc_loader/0.1");
	curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, rpc_loader_impl_write_data);
	curl_easy_setopt(easy, CURLOPT_WRITEDATA, static_cast<loader_impl_rpc_write_data>(&async_ctx->write_data));
	curl_easy_setopt(easy, CURLOPT_PRIVATE, async_ctx);
	curl_easy_setopt(easy, CURLOPT_TIMEOUT_MS, rpc_function->timeout_ms);

	/* COPYPOSTFIELDS copies data internally, safe to free buffer after */
	curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, (long)(body_request_size - 1));
	curl_easy_setopt(easy, CURLOPT_COPYPOSTFIELDS, buffer);

	async_ctx->easy = easy;

	/* Free serialization buffer */
	metacall::metacall_allocator_free(rpc_impl->allocator, buffer);

	/* Enqueue for poll thread (lock-free, wait-free) */
	rpc_impl->async_queue.enqueue(async_ctx);

	/* Wake poll thread from curl_multi_poll (thread-safe) */
	curl_multi_wakeup(rpc_impl->async_multi);

	/* TODO: Implement future return? */
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
	/* TODO: Move this to loader_impl by passing the structure and loader_impl_derived callback */

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
			if (metacall::detail::loader_impl_type_define(impl, type_name(t), t) != 0)
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

	const char *env_timeout = std::getenv("METACALL_RPC_TIMEOUT_MS");
	rpc_impl->timeout_ms = env_timeout ? std::atol(env_timeout) : 30000L;

	const char *env_retry = std::getenv("METACALL_RPC_RETRY_COUNT");
	rpc_impl->retry_count = env_retry ? std::atoi(env_retry) : 15;

	const char *env_delay = std::getenv("METACALL_RPC_RETRY_DELAY_MS");
	rpc_impl->retry_delay_ms = env_delay ? std::atol(env_delay) : 2000L;

	log_write("metacall", metacall::detail::LOG_LEVEL_DEBUG, "RPC loader initialized with timeout=%ldms, retry=%d, delay=%ldms",
		rpc_impl->timeout_ms, rpc_impl->retry_count, rpc_impl->retry_delay_ms);

	struct metacall::metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	rpc_impl->allocator = metacall_allocator_create(metacall::METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	rpc_impl->cached_serial = metacall::detail::serial_create(metacall::metacall_serial());

	if (rpc_impl->allocator == NULL)
	{
		log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Could not create allocator for serialization");

		delete rpc_impl;

		return NULL;
	}

	/* Skip OPENSSL_init_crypto use-of-uninitialized-value from heap allocation of CRYPTO_malloc in uninstrumented libcrypto */
	memory_sanitizer_uninstrumented({
		curl_global_init(CURL_GLOBAL_ALL);
	});

	/* Initialize discover CURL object */
	rpc_impl->discover_curl = curl_easy_init();

	if (rpc_impl->discover_curl == NULL)
	{
		log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Could not create CURL inspect object");

		metacall::metacall_allocator_destroy(rpc_impl->allocator);

		delete rpc_impl;

		return NULL;
	}

	curl_easy_setopt(rpc_impl->discover_curl, CURLOPT_VERBOSE, CURL_VERBOSE);
	curl_easy_setopt(rpc_impl->discover_curl, CURLOPT_HEADER, 0L);
	curl_easy_setopt(rpc_impl->discover_curl, CURLOPT_WRITEFUNCTION, rpc_loader_impl_write_data);

	rpc_impl->headers = NULL;
	rpc_impl->headers = curl_slist_append(rpc_impl->headers, "Accept: application/json");
	rpc_impl->headers = curl_slist_append(rpc_impl->headers, "Content-Type: application/json");
	rpc_impl->headers = curl_slist_append(rpc_impl->headers, "charset: utf-8");

	/* Initialize async multi handle */
	rpc_impl->async_multi = curl_multi_init();

	if (rpc_impl->async_multi == NULL)
	{
		log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Could not create CURL multi handle for async");

		curl_easy_cleanup(rpc_impl->discover_curl);
		metacall::metacall_allocator_destroy(rpc_impl->allocator);
		delete rpc_impl;

		return NULL;
	}

	/* Start poll thread for async transfers */
	rpc_impl->exit_flag.store(false);
	rpc_impl->poll_thread = std::thread(rpc_poll_loop, rpc_impl);

	if (rpc_loader_impl_initialize_types(impl, rpc_impl) != 0)
	{
		log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Could not create CURL object");

		rpc_impl->exit_flag.store(true);
		curl_multi_wakeup(rpc_impl->async_multi);
		rpc_impl->poll_thread.join();

		curl_multi_cleanup(rpc_impl->async_multi);
		curl_easy_cleanup(rpc_impl->discover_curl);
		metacall::metacall_allocator_destroy(rpc_impl->allocator);
		delete rpc_impl;

		return NULL;
	}

	/* Register initialization */
	metacall::detail::loader_initialization_register(impl);

	return rpc_impl;
}

int rpc_loader_impl_execution_path(loader_impl impl, const loader_path path)
{
	loader_impl_rpc rpc_impl = static_cast<loader_impl_rpc>(metacall::detail::loader_impl_get(impl));

	auto pair = rpc_impl->execution_paths.insert(path);

	return pair.second == true ? 0 : 1;
}

loader_impl_rpc_handle rpc_loader_impl_handle_create(std::vector<std::string> buffers)
{
	loader_impl_rpc_handle handle = new loader_impl_rpc_handle_type();

	if (handle == nullptr)
	{
		return nullptr;
	}

	metacall::metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };
	void *allocator = metacall_allocator_create(metacall::METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	for (const auto &buffer : buffers)
	{
		void *json_value = metacall::metacall_deserialize(metacall::metacall_serial(), buffer.c_str(), buffer.length() + 1, allocator);
		handle->configs.emplace_back(json_value);
	}

	return handle;
}

int rpc_loader_impl_load_from_file_read(const loader_path path, std::string &buffer)
{
	std::fstream file;

	file.open(path, std::ios::in | std::ios::binary);

	if (!file.is_open())
	{
		return 1;
	}

	file.seekg(0, std::ios::end);
	buffer.resize(static_cast<std::size_t>(file.tellg()));
	file.seekg(0, std::ios::beg);

	file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

	file.close();

	return file.fail() ? 1 : 0;
}

loader_handle rpc_loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size, void *data)
{
	loader_impl_rpc rpc_impl = static_cast<loader_impl_rpc>(metacall::detail::loader_impl_get(impl));
	std::vector<std::string> buffers;

	(void)data;

	for (size_t iterator = 0; iterator < size; ++iterator)
	{
		const std::string path_str(paths[iterator]);
		size_t path_str_size = path_str.length() + 1;
		std::string buffer;

		if (portability_path_is_absolute(path_str.c_str(), path_str_size) == 0)
		{
			if (rpc_loader_impl_load_from_file_read(paths[iterator], buffer) != 0)
			{
				log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Failed to load RPC configuration from %s", paths[iterator]);
				return NULL;
			}
		}
		else
		{
			bool found = false;

			for (const auto &execution_path : rpc_impl->execution_paths)
			{
				loader_path absolute_path = {};

				(void)portability_path_join(execution_path.c_str(), execution_path.length() + 1, path_str.c_str(), path_str_size, absolute_path, LOADER_PATH_SIZE);

				if (rpc_loader_impl_load_from_file_read(absolute_path, buffer) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Failed to load RPC configuration from %s", paths[iterator]);
				return NULL;
			}
		}

		buffers.push_back(buffer);
	}

	return static_cast<loader_handle>(rpc_loader_impl_handle_create(buffers));
}

loader_handle rpc_loader_impl_load_from_memory(loader_impl impl, const loader_name name, const char *buffer, size_t size, void *data)
{
	std::string buffer_str(buffer, size - 1);
	std::vector<std::string> buffers;

	(void)impl;
	(void)name;
	(void)data;

	buffers.push_back(buffer_str);

	return static_cast<loader_handle>(rpc_loader_impl_handle_create(buffers));
}

loader_handle rpc_loader_impl_load_from_package(loader_impl impl, const loader_path path, void *data)
{
	/* TODO */

	(void)impl;
	(void)path;
	(void)data;

	return NULL;
}

int rpc_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_rpc_handle rpc_handle = static_cast<loader_impl_rpc_handle>(handle);

	(void)impl;

	rpc_handle->configs.clear();

	delete rpc_handle;

	return 0;
}

int rpc_loader_impl_discover_value(loader_impl_rpc rpc_impl, const std::string &url, void *v, context ctx)
{
	metacall::map_typed<std::string, metacall::array> inspect(v);

	for (const auto &[tag, handles] : inspect)
	{
		for (const auto &handle : handles)
		{
			const auto &handle_map = handle.as<metacall::map_typed<std::string, metacall::value>>();
			const auto &scope_map = handle_map["scope"].as<metacall::map_typed<std::string, metacall::value>>();

			for (const auto &func : scope_map["funcs"].as<metacall::array>())
			{
				const auto &func_map = func.as<metacall::map_typed<std::string, metacall::value>>();
				const auto &func_name = func_map["name"].as<std::string>();
				const auto &is_async = func_map["async"].as<bool>();
				const auto &signature_map = func_map["signature"].as<metacall::map_typed<std::string, metacall::value>>();
				const auto &args = signature_map["args"].as<metacall::array>();

				loader_impl_rpc_function rpc_func = new loader_impl_rpc_function_type(rpc_impl, url, is_async, func_name);

				function f = function_create(func_name.c_str(), args.size(), rpc_func, &function_rpc_singleton);
				signature s = function_signature(f);

				function_async(f, is_async ? ASYNCHRONOUS : SYNCHRONOUS);

				size_t arg = 0;

				for (const auto &arg_value : args)
				{
					const auto &arg_map = arg_value.as<metacall::map_typed<std::string, metacall::value>>();
					const auto &type_map = arg_map["type"].as<metacall::map_typed<std::string, metacall::value>>();
					const type_id id = static_cast<type_id>(type_map["id"].as<int>());

					signature_set(s, arg++, arg_map["name"].as<std::string>().c_str(), rpc_impl->types[id]);
				}

				const auto &ret_map = signature_map["ret"].as<metacall::map_typed<std::string, metacall::value>>();
				const auto &type_map = ret_map["type"].as<metacall::map_typed<std::string, metacall::value>>();

				const type_id id = static_cast<type_id>(type_map["id"].as<int>());

				signature_set_return(s, rpc_impl->types[id]);

				scope sp = context_scope(ctx);
				value fn_value = value_create_function(f);

				if (scope_define(sp, function_name(f), fn_value) != 0)
				{
					metacall::metacall_value_destroy(fn_value);
					return 1;
				}
			}
		}
	}

	return 0;
}

int rpc_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_rpc rpc_impl = static_cast<loader_impl_rpc>(metacall::detail::loader_impl_get(impl));
	loader_impl_rpc_handle rpc_handle = static_cast<loader_impl_rpc_handle>(handle);

	for (const auto &config : rpc_handle->configs)
	{
		auto urls = config["urls"].as<metacall::array>();

		for (const auto &url : urls)
		{
			const auto &url_str = url.as<std::string>();
			std::string base_url = url_str.back() != '/' ? url_str + '/' : url_str;
			std::string inspect_url = base_url + "inspect";

			CURLcode res = CURLE_OK;
			bool discovered = false;

			for (int attempt = 0; attempt < rpc_impl->retry_count; ++attempt)
			{
				loader_impl_rpc_write_data_type write_data;

				curl_easy_setopt(rpc_impl->discover_curl, CURLOPT_URL, inspect_url.c_str());
				curl_easy_setopt(rpc_impl->discover_curl, CURLOPT_WRITEDATA, static_cast<loader_impl_rpc_write_data>(&write_data));
				curl_easy_setopt(rpc_impl->discover_curl, CURLOPT_TIMEOUT_MS, rpc_impl->timeout_ms);

				/* Skip curl_multi_perform use-of-uninitialized-value from heap allocation of curl_mvaprintf in uninstrumented libcurl */
				memory_sanitizer_uninstrumented({
					res = curl_easy_perform(rpc_impl->discover_curl);
				});

				if (res == CURLE_OK)
				{
					/* Deserialize the inspect data */
					const size_t size = write_data.buffer.length() + 1;

					void *inspect_value = serial_deserialize(rpc_impl->cached_serial, write_data.buffer.c_str(), size, (metacall::detail::memory_allocator)rpc_impl->allocator);

					if (inspect_value == NULL)
					{
						log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Could not deserialize the inspect data from API endpoint %s", url_str.c_str());
						return 1;
					}

					/* Discover the functions from the inspect value */
					if (rpc_loader_impl_discover_value(rpc_impl, base_url, inspect_value, ctx) != 0)
					{
						log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Invalid inspect value discover from API endpoint %s", url_str.c_str());
						return 1;
					}

					discovered = true;
					break;
				}

				log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Discover attempt %d/%d failed for %s [%s]",
					attempt + 1, rpc_impl->retry_count, url_str.c_str(), curl_easy_strerror(res));

				if (attempt + 1 < rpc_impl->retry_count)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(rpc_impl->retry_delay_ms));
				}
			}

			if (!discovered)
			{
				log_write("metacall", metacall::detail::LOG_LEVEL_ERROR, "Gave up discovering endpoint %s after %d attempts",
					url_str.c_str(), rpc_impl->retry_count);
				return 1;
			}
		}
	}

	return 0;
}

int rpc_loader_impl_destroy(loader_impl impl)
{
	loader_impl_rpc rpc_impl = static_cast<loader_impl_rpc>(metacall::detail::loader_impl_get(impl));

	/* Destroy children loaders */
	metacall::detail::loader_unload_children(impl);

	/* Stop the poll thread, set exit flag, wake it, wait for drain */
	rpc_impl->exit_flag.store(true);
	curl_multi_wakeup(rpc_impl->async_multi);
	if (rpc_impl->poll_thread.joinable())
	{
		rpc_impl->poll_thread.join();
	}

	/* Skip curl_multi_cleanup use-of-uninitialized-value from heap allocation in uninstrumented libcurl */
	memory_sanitizer_uninstrumented({
		/* Clean up async multi and discover handle */
		curl_multi_cleanup(rpc_impl->async_multi);
		curl_easy_cleanup(rpc_impl->discover_curl);
	});

	curl_slist_free_all(rpc_impl->headers);

	metacall::metacall_allocator_destroy(rpc_impl->allocator);

	curl_global_cleanup();

	delete rpc_impl;

	return 0;
}