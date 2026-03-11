/*
 *	CLI Core Plugin by Parra Studios
 *	A plugin implementing core functionality for MetaCall CLI.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <algorithm>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
/* Use ASCII borders on Windows so help output aligns correctly in console */
#define CLI_BOX_TL "\t+"
#define CLI_BOX_TR "+"
#define CLI_BOX_BL "\t+"
#define CLI_BOX_BR "+"
#define CLI_BOX_H  "--------------------------------------------------------------------------------------------"
#define CLI_BOX_V "\t|"
#define CLI_BOX_VR "|"
#else
#define CLI_BOX_TL "\t┌"
#define CLI_BOX_TR "┐"
#define CLI_BOX_BL "\t└"
#define CLI_BOX_BR "┘"
#define CLI_BOX_H  "────────────────────────────────────────────────────────────────────────────────────────"
#define CLI_BOX_V "\t│"
#define CLI_BOX_VR "│"
#endif

/* Error messages */
#define LOAD_ERROR		"Failed to load a script"
#define INSPECT_ERROR	"Failed to inspect MetaCall context"
#define EVAL_ERROR		"Failed to evaluate the expression"
#define CALL_ERROR		"Failed to call the function"
#define AWAIT_ERROR		"Failed to await the function"
#define CLEAR_ERROR		"Failed to clear the handle"
#define COPYRIGHT_ERROR "Failed to show the copyright"
#define HELP_ERROR		"Failed to show the help"
#define DEBUG_ERROR		"Failed to debug the command"

#define COPYRIGHT_PRINT() \
	do \
	{ \
		std::cout << "MetaCall Command Line Interface by Parra Studios" << std::endl; \
		std::cout << "A command line interface for MetaCall Core" << std::endl; \
		std::cout << std::endl; \
		std::cout << "Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>" << std::endl; \
	} while (0)

template <typename result_type, typename cast_func_type>
void *extension_function_check_array(const char *error, void *v, std::vector<result_type> &array, cast_func_type &cast_func, enum metacall_value_id id)
{
	size_t size = metacall_value_count(v);
	void **values = metacall_value_to_array(v);

	array.reserve(size);

	for (size_t i = 0; i < size; ++i)
	{
		if (metacall_value_id(values[i]) != id)
		{
			std::stringstream ss;

			ss << error << ", calling with wrong type of argument at argument position #"
			   << (i + 1) << ", expected " << metacall_value_id_name(id)
			   << ", got " << metacall_value_type_name(values[i]);

			EXTENSION_FUNCTION_THROW(ss.str().c_str());
		}

		array.push_back(cast_func(values[i]));
	}

	return NULL;
}

static const std::vector<std::string> loader_tags(void)
{
	static const std::vector<std::string> loaders = {
		"mock", "py", "node", "rb", "cs", "cob",
		"ts", "js", "file", "wasm", "rs", "c",
		"rpc", "ext", "java"
	};

	return loaders;
}

static bool loader_tag_is_valid(const char *tag)
{
	static const std::vector<std::string> loaders = loader_tags();

	/* Check if invalid loader tag */
	return std::find(std::begin(loaders), std::end(loaders), tag) != std::end(loaders);
}

static void *throw_invalid_tag(const char *error, const char *tag)
{
	static const std::vector<std::string> loaders = loader_tags();
	auto it = std::begin(loaders);
	std::stringstream ss;
	ss << error << ", " << tag << " is not a valid tag, use: " << *it;

	do
	{
		++it;
		ss << ", " << *it;
	} while (it != std::end(loaders));

	EXTENSION_FUNCTION_THROW(ss.str().c_str());
}

static void value_array_for_each(void *v, const std::function<void(void *)> &lambda)
{
	void **v_array = static_cast<void **>(metacall_value_to_array(v));
	size_t count = metacall_value_count(v);

	std::for_each(v_array, v_array + count, lambda);
}

static void value_map_for_each(void *v, const std::function<void(const char *, void *)> &lambda)
{
	void **v_map = static_cast<void **>(metacall_value_to_map(v));
	size_t count = metacall_value_count(v);

	std::for_each(v_map, v_map + count, [&lambda](void *element) {
		void **v_element = metacall_value_to_array(element);
		lambda(metacall_value_to_string(v_element[0]), v_element[1]);
	});
}

static void *parse_function_call(const char *error, void *func_str_value, void **func, std::string &func_args)
{
	std::string func_str = metacall_value_to_string(func_str_value);

	if (func_str.find('(') == std::string::npos || func_str.find(')') == std::string::npos)
	{
		std::stringstream ss;

		ss << error << ", calling function with malformed function call string: " << func_str;

		EXTENSION_FUNCTION_THROW(ss.str().c_str());
	}

	std::string::size_type first_parenthesis = func_str.find_first_of('(');
	std::string::size_type last_parenthesis = func_str.find_first_of(')');
	std::string func_name = func_str.substr(0, first_parenthesis);

	if (first_parenthesis != func_name.size() || last_parenthesis != (func_str.size() - 1))
	{
		std::stringstream ss;

		ss << error << ", calling function with malformed parameters: " << func_str;

		EXTENSION_FUNCTION_THROW(ss.str().c_str());
	}

	/* Convert arguments into an array */
	func_args = "[";
	func_args += func_str.substr(first_parenthesis + 1, func_str.size() - first_parenthesis - 2);
	func_args += "]";

	/* Check if function is loaded */
	const char *func_name_str = const_cast<const char *>(func_name.c_str());
	*func = metacall_function(func_name_str);

	if (*func == NULL)
	{
		std::stringstream ss;

		ss << error << ", function '" << func_name << "' does not exist";

		EXTENSION_FUNCTION_THROW(ss.str().c_str());
	}

	return NULL;
}

void *load(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(LOAD_ERROR, METACALL_STRING, METACALL_ARRAY);

	char *tag = metacall_value_to_string(args[0]);

	if (!loader_tag_is_valid(tag))
	{
		return throw_invalid_tag(LOAD_ERROR, tag);
	}

	std::vector<std::string> scripts;
	void *error = extension_function_check_array(LOAD_ERROR, args[1], scripts, metacall_value_to_string, METACALL_STRING);

	if (error != NULL)
	{
		return error;
	}

	std::vector<const char *> scripts_cstr;

	scripts_cstr.reserve(scripts.size());

	for (auto it = std::begin(scripts); it != std::end(scripts); ++it)
	{
		scripts_cstr.push_back((*it).c_str());
	}

	int ret = metacall_load_from_file(tag, scripts_cstr.data(), scripts_cstr.size(), NULL);

	return metacall_value_create_int(ret);
}

void *inspect(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(INSPECT_ERROR);

	void *v = metacall_inspect_value();

	if (v == NULL)
	{
		EXTENSION_FUNCTION_THROW(INSPECT_ERROR ", inspect returned invalid value");
	}

	/* Print run-times */
	value_map_for_each(v, [](const char *key, void *modules) {
		std::cout << "runtime " << key;

		if (metacall_value_count(modules) == 0)
		{
			std::cout << std::endl;
			return;
		}

		std::cout << " {" << std::endl;

		/* Print scripts */
		value_array_for_each(modules, [](void *module) {
			/* Get module name */
			void **v_module_map = static_cast<void **>(metacall_value_to_map(module));
			void **v_module_name_tuple = metacall_value_to_array(v_module_map[0]);
			const char *name = metacall_value_to_string(v_module_name_tuple[1]);

			std::cout << "\tmodule " << name << " { " << std::endl;

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

					std::cout << "\t\tfunction " << func_name << "(";

					/* Get function signature */
					void **v_signature_tuple = metacall_value_to_array(v_func_map[1]);
					void **v_args_map = metacall_value_to_map(v_signature_tuple[1]);
					void **v_args_tuple = metacall_value_to_array(v_args_map[1]);
					void **v_args_array = metacall_value_to_array(v_args_tuple[1]);

					size_t iterator = 0, count = metacall_value_count(v_args_tuple[1]);

					value_array_for_each(v_args_array, [&iterator, &count](void *arg) {
						void **v_arg_map = metacall_value_to_map(arg);
						void **v_arg_name_tupla = metacall_value_to_array(v_arg_map[0]);
						std::string parameter_name(metacall_value_to_string(v_arg_name_tupla[1]));

						if (parameter_name.empty())
						{
							parameter_name += "arg";
							parameter_name += std::to_string(iterator);
						}

						std::cout << parameter_name;

						if (iterator + 1 < count)
						{
							std::cout << ", ";
						}

						++iterator;
					});

					std::cout << ")" << std::endl;
				});
			}

			std::cout << "\t}" << std::endl;
		});

		std::cout << "}" << std::endl;
	});

	metacall_value_destroy(v);

	return metacall_value_create_int(0);
}

void *eval(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(EVAL_ERROR, METACALL_STRING, METACALL_STRING);

	char *tag = metacall_value_to_string(args[0]);

	if (!loader_tag_is_valid(tag))
	{
		return throw_invalid_tag(EVAL_ERROR, tag);
	}

	char *script = metacall_value_to_string(args[1]);
	size_t size = metacall_value_size(args[1]);

	return metacall_value_create_int(metacall_load_from_memory(tag, script, size, NULL));
}

void *call(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(CALL_ERROR, METACALL_STRING);

	/* Parse function call */
	void *func = NULL;
	std::string func_args;
	void *error = parse_function_call(CALL_ERROR, args[0], &func, func_args);

	if (error != NULL)
	{
		return error;
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
	void *func = NULL;
	std::string func_args;
	void *error = parse_function_call(AWAIT_ERROR, args[0], &func, func_args);

	if (error != NULL)
	{
		return error;
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

	if (!loader_tag_is_valid(tag))
	{
		return throw_invalid_tag(CLEAR_ERROR, tag);
	}

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

void *copyright(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(COPYRIGHT_ERROR);

	COPYRIGHT_PRINT();

	return metacall_value_create_int(0);
}

void *help(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(HELP_ERROR);

	/* Print copyright first */
	COPYRIGHT_PRINT();

	/* Command list */
	std::cout << std::endl
			  << "Command list:" << std::endl
			  << std::endl;

	/* Load command */
	std::cout << CLI_BOX_TL << CLI_BOX_H << CLI_BOX_TR << std::endl;
	std::cout << CLI_BOX_V << "Load a script from file into MetaCall                                                  " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << CLI_BOX_H << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Usage:                                                                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "load <runtime tag> <script0> <script1> ... <scriptN>                                   " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "   <runtime tag> : identifier to the type of script                                    " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                 options :                                                             " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           mock - Mock (for testing purposes)                          " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           py   - Python                                               " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           node - NodeJS                                               " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           rb   - Ruby                                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           cs   - C# NetCore                                           " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           cob  - Cobol                                                " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           ts   - TypeScript                                           " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           js   - V8 JavaScript Engine                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           file - Files (for handling file systems)                    " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "   <script0> <script1> ... <scriptN> : relative or absolute path to the script(s)      " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                                                                       " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Example:                                                                               " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "load node concat.js                                                                    " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "load py example.py                                                                     " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "load rb hello.rb                                                                       " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                                                                       " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Result:                                                                                " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Script (concat.js) loaded correctly                                                    " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_BL << CLI_BOX_H << CLI_BOX_BR << std::endl
			  << std::endl;

	/* Inspect command */
	std::cout << CLI_BOX_TL << CLI_BOX_H << CLI_BOX_TR << std::endl;
	std::cout << CLI_BOX_V << "Show all runtimes, modules and functions (with their signature) loaded into MetaCall   " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << CLI_BOX_H << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Usage:                                                                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "inspect                                                                                " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                                                                       " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Example:                                                                               " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "inspect                                                                                " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                                                                       " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Result:                                                                                " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "runtime node {                                                                         " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "    module concat {                                                                    " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "        function concat(left, right)                                                   " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "    }                                                                                  " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "}                                                                                      " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_BL << CLI_BOX_H << CLI_BOX_BR << std::endl
			  << std::endl;

	/* Eval command */
	std::cout << CLI_BOX_TL << CLI_BOX_H << CLI_BOX_TR << std::endl;
	std::cout << CLI_BOX_V << "Evaluate a code snippet with the specified runtime tag                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << CLI_BOX_H << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Usage:                                                                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "eval <runtime tag> <script>                                                            " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "   <runtime tag> : identifier to the type of script                                    " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                 options :                                                             " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           mock - Mock (for testing purposes)                          " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           py   - Python                                               " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           node - NodeJS                                               " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           rb   - Ruby                                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           cs   - C# NetCore                                           " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           cob  - Cobol                                                " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           ts   - TypeScript                                           " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           js   - V8 JavaScript Engine                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           file - Files (for handling file systems)                    " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "   <script> : textual code to execute                                                  " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                                                                       " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Example:                                                                               " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "eval node console.log(\"hello world\")                                                   " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                                                                       " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Result:                                                                                " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "\"hello world\"                                                                          " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_BL << CLI_BOX_H << CLI_BOX_BR << std::endl
			  << std::endl;

	/* Call command */
	std::cout << CLI_BOX_TL << CLI_BOX_H << CLI_BOX_TR << std::endl;
	std::cout << CLI_BOX_V << "Call a function previously loaded in MetaCall                                          " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << CLI_BOX_H << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Usage:                                                                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "call <function name>(<arg0>, <arg1>, ... , <argN>)                                     " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "   <function name> : alphanumeric string beginning by letter (method1, method2, hello) " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "   <arg0>, <arg1>, ... , <argN>  : arguments to be passed to the function in JSON      " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                   types :                                             " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                           bool   - true, false                        " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                           number - 5, 4.34                            " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                           string - \"hello world\"                      " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                           array  - [2, true, \"abc\"]                   " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                           object - { \"one\": 1, \"two\": 2 }             " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                                                                       " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Example:                                                                               " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "call concat(\"hello\", \"world\")                                                          " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                                                                       " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Result:                                                                                " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "\"hello world\"                                                                          " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_BL << CLI_BOX_H << CLI_BOX_BR << std::endl
			  << std::endl;

	/* Await command */
	std::cout << CLI_BOX_TL << CLI_BOX_H << CLI_BOX_TR << std::endl;
	std::cout << CLI_BOX_V << "Await an async function previously loaded in MetaCall                                  " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << CLI_BOX_H << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Usage:                                                                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "await <function name>(<arg0>, <arg1>, ... , <argN>)                                    " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "   <function name> : alphanumeric string beginning by letter (method1, method2, hello) " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "   <arg0>, <arg1>, ... , <argN>  : arguments to be passed to the function in JSON      " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                   types :                                             " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                           bool   - true, false                        " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                           number - 5, 4.34                            " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                           string - \"hello world\"                      " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                           array  - [2, true, \"abc\"]                   " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                           object - { \"one\": 1, \"two\": 2 }             " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                                                                       " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Example:                                                                               " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "await concat(\"hello\", \"world\")                                                         " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                                                                       " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Result:                                                                                " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "\"hello world\"                                                                          " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_BL << CLI_BOX_H << CLI_BOX_BR << std::endl
			  << std::endl;

	/* Clear command */
	std::cout << CLI_BOX_TL << CLI_BOX_H << CLI_BOX_TR << std::endl;
	std::cout << CLI_BOX_V << "Delete a script previously loaded in MetaCall                                          " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << CLI_BOX_H << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Usage:                                                                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "clear <runtime tag> <script0> <script1> ... <scriptN>                                  " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "   <runtime tag> : identifier to the type of script                                    " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                 options :                                                             " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           mock - Mock (for testing purposes)                          " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           py   - Python                                               " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           node - NodeJS                                               " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           rb   - Ruby                                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           cs   - C# NetCore                                           " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           cob  - Cobol                                                " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           ts   - TypeScript                                           " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           js   - V8 JavaScript Engine                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                           file - Files (for handling file systems)                    " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "   <script0> <script1> ... <scriptN> : id of the script (file name without extension)  " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                                                                       " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Example:                                                                               " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "clear node concat                                                                      " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "                                                                                       " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Result:                                                                                " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Script (concat) removed correctly                                                      " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_BL << CLI_BOX_H << CLI_BOX_BR << std::endl
			  << std::endl;

	/* Exit command */
	std::cout << CLI_BOX_TL << CLI_BOX_H << CLI_BOX_TR << std::endl;
	std::cout << CLI_BOX_V << "Exit from MetaCall CLI                                                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << CLI_BOX_H << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Usage:                                                                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "exit                                                                                   " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_BL << CLI_BOX_H << CLI_BOX_BR << std::endl
			  << std::endl;

	/* Copyright command */
	std::cout << CLI_BOX_TL << CLI_BOX_H << CLI_BOX_TR << std::endl;
	std::cout << CLI_BOX_V << "Show copyright of MetaCall CLI                                                         " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << CLI_BOX_H << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Usage:                                                                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "copyright                                                                              " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_BL << CLI_BOX_H << CLI_BOX_BR << std::endl
			  << std::endl;

	/* Help command */
	std::cout << CLI_BOX_TL << CLI_BOX_H << CLI_BOX_TR << std::endl;
	std::cout << CLI_BOX_V << "Show help for MetaCall CLI                                                             " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << CLI_BOX_H << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "Usage:                                                                                 " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_V << "help                                                                                   " << CLI_BOX_VR << std::endl;
	std::cout << CLI_BOX_BL << CLI_BOX_H << CLI_BOX_BR << std::endl
			  << std::endl;

	return metacall_value_create_int(0);
}

void *debug(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(DEBUG_ERROR, METACALL_ARRAY);

	std::vector<std::string> tokens;

	void *error = extension_function_check_array(DEBUG_ERROR, args[0], tokens, metacall_value_to_string, METACALL_STRING);

	if (error != NULL)
	{
		return error;
	}

	std::stringstream ss;

	ss << "{" << std::endl;

	for (size_t position = 0; position < tokens.size(); ++position)
	{
		ss << "\t[" << position << "]: " << tokens[position] << std::endl;
	}

	ss << "}";

	std::string result = ss.str();

	return metacall_value_create_string(result.c_str(), result.length());
}

int cli_core_plugin(void *loader, void *handle)
{
	/* Register functions */
	EXTENSION_FUNCTION(METACALL_INT, load, METACALL_STRING, METACALL_ARRAY);
	EXTENSION_FUNCTION(METACALL_INT, inspect);
	EXTENSION_FUNCTION(METACALL_INT, eval, METACALL_STRING, METACALL_STRING);
	EXTENSION_FUNCTION(METACALL_PTR, call, METACALL_STRING);
	EXTENSION_FUNCTION(METACALL_PTR, await, METACALL_STRING);
	EXTENSION_FUNCTION(METACALL_INT, clear, METACALL_STRING, METACALL_STRING);
	EXTENSION_FUNCTION(METACALL_INT, copyright);
	EXTENSION_FUNCTION(METACALL_INT, help);
	EXTENSION_FUNCTION(METACALL_STRING, debug, METACALL_ARRAY);

	return 0;
}
