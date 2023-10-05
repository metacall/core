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

#include <plugin/plugin_interface.hpp>

#include <condition_variable>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

/* Error messages */
#define LOAD_ERROR	  "Failed to load a script"
#define INSPECT_ERROR "Failed to inspect MetaCall context"
#define EVAL_ERROR	  "Failed to evaluate the expression"
#define CALL_ERROR	  "Failed to call the function"
#define AWAIT_ERROR	  "Failed to await the function"
#define CLEAR_ERROR	  "Failed to clear the handle"

void *load(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(LOAD_ERROR, METACALL_STRING, METACALL_ARRAY);

	char *tag = metacall_value_to_string(args[0]);
	size_t size = metacall_value_count(args[1]);
	void **script_values = metacall_value_to_array(args[1]);
	std::vector<std::string> scripts;
	std::vector<const char *> scripts_cstr;

	scripts.reserve(size);
	scripts_cstr.reserve(size);

	for (size_t i = 0; i < size; ++i)
	{
		if (metacall_value_id(script_values[i]) != METACALL_STRING)
		{
			std::stringstream ss;

			ss << LOAD_ERROR ", calling load with wrong type of argument at argument position "
			   << (i + 1) << ", expected " << metacall_value_id_name(METACALL_STRING)
			   << ", got " << metacall_value_type_name(script_values[i]);

			EXTENSION_FUNCTION_THROW(ss.str().c_str());
		}

		scripts.push_back(metacall_value_to_string(script_values[i]));
		scripts_cstr.push_back(const_cast<const char *>(scripts[i].c_str()));
	}

	int ret = metacall_load_from_file(tag, scripts_cstr.data(), size, NULL);

	return metacall_value_create_int(ret);
}

void *inspect(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(INSPECT_ERROR);

	size_t size = 0;

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };
	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	char *inspect_str = metacall_inspect(&size, allocator);

	void *inspect_value_str = metacall_value_create_string(inspect_str, size > 0 ? size - 1 : 0);

	metacall_allocator_free(allocator, inspect_str);

	metacall_allocator_destroy(allocator);

	return inspect_value_str;
}

void *eval(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(EVAL_ERROR, METACALL_STRING, METACALL_STRING);

	char *tag = metacall_value_to_string(args[0]);
	char *script = metacall_value_to_string(args[1]);
	size_t size = metacall_value_size(args[1]);

	return metacall_value_create_int(metacall_load_from_memory(tag, script, size, NULL));
}

void *call(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(CALL_ERROR, METACALL_STRING);

	/* Parse function call */
	std::string func_str = metacall_value_to_string(args[0]);

	if (func_str.find('(') == std::string::npos || func_str.find(')') == std::string::npos)
	{
		std::stringstream ss;

		ss << CALL_ERROR ", calling function with malformed function call string: " << func_str;

		EXTENSION_FUNCTION_THROW(ss.str().c_str());
	}

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
		std::stringstream ss;

		ss << CALL_ERROR ", function '" << func_name << "' does not exist";

		EXTENSION_FUNCTION_THROW(ss.str().c_str());
	}

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };
	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	void *result = metacallfs(func, func_args.c_str(), func_args.length() + 1, allocator);

	metacall_allocator_destroy(allocator);

	return result;
}

void *await(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(CALL_ERROR, METACALL_STRING);

	/* Parse function call */
	std::string func_str = metacall_value_to_string(args[0]);

	if (func_str.find('(') == std::string::npos || func_str.find(')') == std::string::npos)
	{
		std::stringstream ss;

		ss << CALL_ERROR ", calling function with malformed function call string: " << func_str;

		EXTENSION_FUNCTION_THROW(ss.str().c_str());
	}

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
		std::stringstream ss;

		ss << CALL_ERROR ", function '" << func_name << "' does not exist";

		EXTENSION_FUNCTION_THROW(ss.str().c_str());
	}

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };
	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	std::mutex await_mutex;				/* Mutex for blocking the process until await is resolved */
	std::condition_variable await_cond; /* Condition to be fired once await method is resolved or rejected */

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

	/* Unused */
	metacall_value_destroy(future);

	metacall_allocator_destroy(allocator);

	return fdata.v;
}

void *clear(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(CLEAR_ERROR, METACALL_STRING, METACALL_STRING);

	char *tag = metacall_value_to_string(args[0]);
	char *script = metacall_value_to_string(args[1]);

	void *handle = metacall_handle(tag, script);

	if (handle == NULL)
	{
		std::stringstream ss;

		ss << CLEAR_ERROR ", handle '" << script << "' was not found in loader with tag: " << tag;

		EXTENSION_FUNCTION_THROW(ss.str().c_str());
	}

	return metacall_value_create_int(metacall_clear(handle));
}

int cli_core_plugin(void *loader, void *handle, void *context)
{
	(void)handle;

	EXTENSION_FUNCTION(METACALL_INT, load, METACALL_STRING, METACALL_ARRAY);
	EXTENSION_FUNCTION(METACALL_STRING, inspect);
	EXTENSION_FUNCTION(METACALL_INT, eval, METACALL_STRING, METACALL_STRING);
	EXTENSION_FUNCTION(METACALL_PTR, call, METACALL_STRING);
	EXTENSION_FUNCTION(METACALL_PTR, await, METACALL_STRING);
	EXTENSION_FUNCTION(METACALL_INT, clear, METACALL_STRING, METACALL_STRING);

	return 0;
}
