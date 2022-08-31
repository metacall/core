/*
 *	CLI Core Plugin by Parra Studios
 *	A plugin implementing core functionality for MetaCall CLI.
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

#include <cli_core_plugin/cli_core_plugin.h>

#include <log/log.h>
#include <metacall/metacall.h>

#include <string.h>

#include <condition_variable>
#include <mutex>
#include <string>
#include <unordered_map>

void *load(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)data;

	char *tag = metacall_value_to_string(args[0]);
	if (tag == NULL)
	{
		return metacall_value_create_int(1);
	}

	size_t size = metacall_value_count(args[1]);

	char **scripts = (char **)malloc(sizeof(char *) * size);
	void **script_val = metacall_value_to_array(args[1]);
	if (scripts == NULL || script_val == NULL)
	{
		return metacall_value_create_int(1);
	}

	for (size_t i = 0; i < size; ++i)
	{
		scripts[i] = metacall_value_to_string(script_val[i]);
	}

	int ret = metacall_load_from_file(tag, const_cast<const char **>(scripts), size, NULL);

	free(scripts);

	return metacall_value_create_int(ret);
}

void *eval(size_t argc, void *args[], void *data)
{
	(void)data;

	if (argc != 2)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Calling eval with wrong number of arguments, expected 2 arguments, got %" PRIuS " arguments", argc);
		return metacall_value_create_int(1);
	}

	char *tag = metacall_value_to_string(args[0]);
	char *script = metacall_value_to_string(args[1]);

	int ret = metacall_load_from_memory(tag, script, strlen(script) + 1, NULL);
	return metacall_value_create_int(ret);
}

void *await(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)data;

	/* Parse function call */
	std::string func_str = metacall_value_to_string(args[0]);

	std::string::size_type idx = func_str.find_first_of('(');
	std::string func_name = func_str.substr(0, idx);

	/* Convert arguments into an array */
	std::string func_args = "[";
	func_args += func_str.substr(idx + 1, func_str.size() - (idx + 2));
	func_args += "]";

	/* Check if function is loaded */
	const char *func_name_str = const_cast<const char *>(func_name.c_str());
	void *func = metacall_function(func_name_str);
	if (func == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Function %s does not exist", func_name_str);
		return metacall_value_create_int(1);
	}

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	std::mutex await_mutex;				/**< Mutex for blocking the process until await is resolved */
	std::condition_variable await_cond; /**< Condition to be fired once await method is resolved or rejected */

	std::unique_lock<std::mutex> lock(await_mutex);

	struct await_data_type
	{
		void *v;
		std::mutex &mutex;
		std::condition_variable &cond;
	};

	struct await_data_type fdata = { NULL, await_mutex, await_cond };

	void *future = metacallfs_await(
		func, func_args.c_str(), func_args.length() + 1, allocator,
		[](void *result, void *ctx) -> void * {
			struct await_data_type *await_data = static_cast<struct await_data_type *>(ctx);
			std::unique_lock<std::mutex> lock(await_data->mutex);
			/* Value must be always copied, it gets deleted after the scope */
			await_data->v = metacall_value_copy(result);
			await_data->cond.notify_one();
			return NULL;
		},
		[](void *result, void *ctx) -> void * {
			struct await_data_type *await_data = static_cast<struct await_data_type *>(ctx);
			std::unique_lock<std::mutex> lock(await_data->mutex);
			/* Value must be always copied, it gets deleted after the scope */
			await_data->v = metacall_value_copy(result);
			await_data->cond.notify_one();
			return NULL;
		},
		static_cast<void *>(&fdata));

	await_cond.wait(lock);

	return fdata.v;
	/* Unused */
	metacall_value_destroy(future);

	metacall_allocator_destroy(allocator);
}

void *call(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)data;

	/* Parse function call */
	std::string func_str = metacall_value_to_string(args[0]);

	std::string::size_type idx = func_str.find_first_of('(');
	std::string func_name = func_str.substr(0, idx);

	/* Convert arguments into an array */
	std::string func_args = "[";
	func_args += func_str.substr(idx + 1, func_str.size() - (idx + 2));
	func_args += "]";

	/* Check if function is loaded */
	const char *func_name_str = const_cast<const char *>(func_name.c_str());
	void *func = metacall_function(func_name_str);
	if (func == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Function %s does not exist", func_name_str);
		return metacall_value_create_int(1);
	}

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	void *result = metacallfs(func, func_args.c_str(), func_args.length() + 1, allocator);

	metacall_allocator_destroy(allocator);
	return result;
}

void *clear(size_t argc, void *args[], void *data)
{
	(void)data;

	if (argc != 2)
	{
		return metacall_value_create_int(1);
	}

	char *tag = metacall_value_to_string(args[0]);
	char *script = metacall_value_to_string(args[1]);

	void *handle = metacall_handle(tag, script);

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Handle %s not found in loader (%s)", script, tag);
		return metacall_value_create_int(1);
	}

	if (metacall_clear(handle) != 0)
	{
		return metacall_value_create_int(1);
	}

	return metacall_value_create_int(0);
}

void *inspect(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)args;
	(void)data;

	size_t size = 0;

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };
	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	char *inspect_str = metacall_inspect(&size, allocator);

	void *inspect_value_str = metacall_value_create_string(inspect_str, size > 0 ? size - 1 : 0);

	metacall_allocator_free(allocator, inspect_str);

	metacall_allocator_destroy(allocator);

	return inspect_value_str;
}

int cli_core_plugin(void *loader, void *handle, void *context)
{
	(void)handle;
	int ret = 0;
	{
		enum metacall_value_id *arg_types = NULL;
		if (metacall_register_loaderv(loader, context, "inspect", inspect, METACALL_STRING, 0, arg_types) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to register function: inspect");
			ret = 1;
		}
	}

	{
		enum metacall_value_id arg_types[] = { METACALL_STRING, METACALL_STRING };
		if (metacall_register_loaderv(loader, context, "clear", clear, METACALL_INT, sizeof(arg_types) / sizeof(arg_types[0]), arg_types) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to register function: clear");
			ret = 1;
		}
	}

	{
		enum metacall_value_id arg_types[] = { METACALL_STRING };
		if (metacall_register_loaderv(loader, context, "call", call, METACALL_PTR, sizeof(arg_types) / sizeof(arg_types[0]), arg_types) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to register function: call");
			ret = 1;
		}
	}

	{
		enum metacall_value_id arg_types[] = { METACALL_STRING };
		if (metacall_register_loaderv(loader, context, "await", await, METACALL_PTR, sizeof(arg_types) / sizeof(arg_types[0]), arg_types) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to register function: await");
			ret = 1;
		}
	}

	{
		enum metacall_value_id arg_types[] = { METACALL_STRING, METACALL_STRING };
		if (metacall_register_loaderv(loader, context, "eval", eval, METACALL_INT, sizeof(arg_types) / sizeof(arg_types[0]), arg_types) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to register function: eval");
			ret = 1;
		}
	}

	{
		enum metacall_value_id arg_types[] = { METACALL_STRING, METACALL_ARRAY };
		if (metacall_register_loaderv(loader, context, "load", load, METACALL_INT, sizeof(arg_types) / sizeof(arg_types[0]), arg_types) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to register function: load");
			ret = 1;
		}
	}

	return ret;
}
