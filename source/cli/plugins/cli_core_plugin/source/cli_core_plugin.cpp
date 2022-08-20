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

#include <algorithm>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>

void *help(size_t argc, void *args[], void *data)
{
	(void)data;
	static const char *help_usage_str =
		("Usage:\n"
		 "      help <option>\n"
		 "      help <command>\n"
		 "options:\n"
		 "         -a               list all commands\n"
		 "         --runtime-tag    list all runtime tags\n\n"

		 "'metacallcli --help' for more general information.\n");

	static const char *command_list_str =
		("commands\n"
		 "eval              Evaluate a code snippet with the specified runtime tag: <tag> <code>\n"
		 "                  loader tag: node, py, c...\n"
		 "load              Load a script from file into MetaCall\n"
		 "call              Call a function previously loaded in MetaCall\n"
		 "inspect           Show all runtimes, modules and functions (with their signature)\n"
		 "                	 loaded into MetaCall\n"

		 "await             Await an async function previously loaded in MetaCall\n"
		 "clear             Delete a script previously loaded in MetaCall\n"
		 "get_function_list Show all loaded functions");

	static const char *tags =
		("tags :\n"
		 "        mock - Mock (for testing purposes)\n"
		 "        py   - Python\n"
		 "        rs   - Rust\n"
		 "        c    - C source files\n"
		 "        ts   - TypeScript, tsx, jsx\n"
		 "        py   - Python\n"
		 "        py   - Python\n"
		 "        py   - Python\n"
		 "        node - NodeJS, Js\n"
		 "        rb   - Ruby\n"
		 "        cs   - C# NetCore\n"
		 "        cob  - Cobol\n"
		 "        js   - V8 JavaScript Engine\n"
		 "        file - Files (for handling file systems)\n");

	/*Concept guides*/
	static const char *load_help =
		("Command: load\n"
		 "Load a script from file into MetaCall\n"
		 "\n"
		 "Usage:\n"
		 "load <runtime tag> <script0> <script1> ... <scriptN>\n"
		 "   <runtime tag> : identifier to the type of script\n"
		 "                 options :\n"
		 "                           mock - Mock (for testing purposes)\n"
		 "                           py   - Python\n"
		 "                           node - NodeJS\n"
		 "                           rb   - Ruby\n"
		 "                           cs   - C# NetCore\n"
		 "                           cob  - Cobol\n"
		 "                           ts   - TypeScript\n"
		 "                           js   - V8 JavaScript Engine\n"
		 "                           file - Files (for handling file systems)\n"
		 "   <script0> <script1> ... <scriptN> : relative or absolute path to the script(s)\n"
		 "\n"
		 "Example:\n"
		 "load node concat.js\n"
		 "load py example.py\n"
		 "load rb hello.rb\n"
		 "\n"
		 "Result:\n"
		 "Script (concat.js) loaded correctly\n");

	static const char *eval_help =
		("Command: eval\n"
		 "Evaluate a code snippet with the specified runtime tag\n"
		 "\n"
		 "Usage:\n"
		 "eval <runtime tag> <script>\n"
		 "   <runtime tag> : identifier to the type of script\n"
		 "                 options :\n"
		 "                           mock - Mock (for testing purposes)\n"
		 "                           py   - Python\n"
		 "                           node - NodeJS\n"
		 "                           rb   - Ruby\n"
		 "                           cs   - C# NetCore\n"
		 "                           cob  - Cobol\n"
		 "                           ts   - TypeScript\n"
		 "                           js   - V8 JavaScript Engine\n"
		 "                           file - Files (for handling file systems)\n"
		 "   <script> : textual code to execute\n"
		 "\n"
		 "Example:\n"
		 "eval node console.log(\"hello world\")\n"
		 "\n"
		 "Result:\n"
		 "\"hello world\"\n");

	static const char *call_help =
		("Command: call\n"
		 "Call a function previously loaded in MetaCall\n"
		 "\n"
		 "Usage:\n"
		 "call <function name>(<arg0>, <arg1>, ... , <argN>)\n"
		 "   <function name> : alphanumeric string beginning by letter (method1, method2, hello)\n"
		 "   <arg0>, <arg1>, ... , <argN>  : arguments to be passed to the function in JSON\n"
		 "                                   types :\n"
		 "                                           bool   - true, false\n"
		 "                                           number - 5, 4.34\n"
		 "                                           string - \"hello world\"\n"
		 "                                           array  - [2, true, \"abc\"]\n"
		 "                                           object - { \"one\": 1, \"two\": 2 }\n"
		 "\n"
		 "Example:\n"
		 "call concat(\"hello\", \"world\")\n"
		 "\n"
		 "Result:\n"
		 "\"hello world\"\n");

	static const char *await_help =
		("Command: await\n"
		 "Await an async function previously loaded in MetaCall\n"
		 "\n"
		 "Usage:\n"
		 "await <function name>(<arg0>, <arg1>, ... , <argN>)\n"
		 "   <function name> : alphanumeric string beginning by letter (method1, method2, hello)"
		 "   <arg0>, <arg1>, ... , <argN>  : arguments to be passed to the function in JSON\n"
		 "                                   types :\n"
		 "                                           bool   - true, false\n"
		 "                                           number - 5, 4.34\n"
		 "                                           string - \"hello world\"\n"
		 "                                           array  - [2, true, \"abc\"]\n"
		 "                                           object - { \"one\": 1, \"two\": 2 }\n"
		 "\n"
		 "Example:\n"
		 "await concat(\"hello\", \"world\")\n"
		 "\n"
		 "Result:\n"
		 "\"hello world\"\n");

	static const char *clear_help =
		("command: clear\n"
		 "Delete a script previously loaded in MetaCall\n"
		 "\n"
		 "Usage:\n"
		 "clear <runtime tag> <script0> <script1> ... <scriptN>\n"
		 "    <runtime tag> : identifier to the type of script\n"
		 "                  options :\n"
		 "                            mock - Mock (for testing purposes)\n"
		 "                            py   - Python\n"
		 "                            node - NodeJS\n"
		 "                            rb   - Ruby\n"
		 "                            cs   - C# NetCore\n"
		 "                            cob  - Cobol\n"
		 "                            ts   - TypeScript\n"
		 "                            js   - V8 JavaScript Engine\n"
		 "                            file - Files (for handling file systems)\n"
		 "    <script0> <script1> ... <scriptN> : id of the script (file name without extension)\n"
		 "\n"
		 "Example:\n"
		 " clear node concat\n"
		 "\n"
		 "Result:\n"
		 "Script (concat) removed correctly\n");

	static const char *inspect_help =
		("Command: inspect\n"
		 "Show all runtimes, modules and functions (with their signature) loaded into MetaCall\n"
		 "\n"
		 "Usage:\n"
		 "inspect\n"
		 "\n"
		 "Example:\n"
		 "inspect\n"
		 "\n"
		 "Result:\n"
		 "runtime node {\n"
		 "  	module concat {\n"
		 "     function concat(left, right)\n"
		 "}\n"
		 "}\n");

	static const char *get_function_list =
		("Command: get_function_list\n"
		 "Show all functions loaded into MetaCall from the repl\n"
		 "\n"
		 "Usage:\n"
		 "get_function_list\n"
		 "\n"
		 "Example:\n"
		 "get_function_list\n"
		 "\n"
		 "Result:\n"
		 "concat(left, right)\n");

	static std::unordered_map<std::string, std::string> help_map = {
		{ "eval", eval_help },
		{ "call", call_help },
		{ "load", load_help },
		{ "clear", clear_help },
		{ "await", await_help },
		{ "inspect", inspect_help },
		{ "get_function_list", get_function_list }
	};

	if (argc == 0)
	{
		return metacall_value_create_string(help_usage_str, strlen(help_usage_str));
	}
	else if (argc > 1)
	{
		std::string err_msg = "'help' called with too many arguments";
		return metacall_value_create_string(err_msg.c_str(), err_msg.size());
	}

	char *option = metacall_value_to_string(args[0]);

	if (!strcmp(option, "-a"))
	{
		return metacall_value_create_string(command_list_str, strlen(command_list_str));
	}
	else if (!strcmp(option, "--runtime-tag"))
	{
		return metacall_value_create_string(tags, strlen(tags));
	}
	else
	{
		if (help_map.find(option) != help_map.end())
		{
			return metacall_value_create_string(help_map[option].c_str(), help_map[option].size());
		}
		else
		{
			std::string err_msg;
			if (option[0] == '-')
			{
				err_msg = std::string("Unknown option: ") + option;
			}
			else
			{
				err_msg = std::string("Unknown command: ") +
						  option + "\n\ttry '" + option + std::string(" --help'");
			}

			return metacall_value_create_string(err_msg.c_str(), err_msg.size());
		}
	}
}

void *load(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)data;

	if (argc < 2)
	{
		return metacall_value_create_int(1);
	}

	void **script_val = NULL;
	char **scripts = NULL;

	char *tag = metacall_value_to_string(args[0]);
	if (tag == NULL)
	{
		return metacall_value_create_int(1);
	}

	size_t size = 0;
	if (metacall_value_id(args[1]) == METACALL_ARRAY)
	{
		size = metacall_value_count(args[1]);
		script_val = metacall_value_to_array(args[1]);
		if (script_val == NULL)
		{
			return metacall_value_create_int(1);
		}
	}
	else
	{
		script_val = (args + 1);
		size = argc - 1;
	}

	scripts = (char **)malloc(sizeof(char *) * size);

	for (size_t i = 0; i < size; ++i)
	{
		if (metacall_value_id(script_val[i]) == METACALL_STRING)
		{
			scripts[i] = metacall_value_to_string(script_val[i]);
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Calling load with wrong type of argument at argument position %" PRIuS ", expected %s, got %s",
				i + 1, metacall_value_id_name(METACALL_STRING), metacall_value_type_name(script_val[i]));
			return metacall_value_create_int(1);
		}
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

	if (metacall_value_id(args[0]) == METACALL_STRING && metacall_value_id(args[1]) == METACALL_STRING)
	{
		char *tag = metacall_value_to_string(args[0]);
		char *script = metacall_value_to_string(args[1]);

		return metacall_value_create_int(metacall_load_from_memory(tag, script, strlen(script) + 1, NULL));
	}
	else
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Calling eval with wrong type of arguments, expected two %s, got %s and %s",
			metacall_value_id_name(METACALL_STRING), metacall_value_type_name(args[0]), metacall_value_type_name(args[1]));
	}

	return metacall_value_create_int(1);
}

void *await(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)data;

	if (argc != 1)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Calling await with wrong number of arguments, expected 1 arguments, got %" PRIuS " arguments", argc);
		return metacall_value_create_int(1);
	}

	if (metacall_value_id(args[0]) != METACALL_STRING)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Calling await with wrong type of arguments, expected %s, got %s",
			metacall_value_id_name(METACALL_STRING), metacall_value_type_name(args[0]));
		return metacall_value_create_int(1);
	}

	/* Parse function call */
	std::string func_str = metacall_value_to_string(args[0]);
	if (func_str.find('(') == std::string::npos || func_str.find(')') == std::string::npos)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "'await' called with mangled function call string: %s", func_str.c_str());
		return metacall_value_create_int(1);
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

	/* Unused */
	metacall_value_destroy(future);

	metacall_allocator_destroy(allocator);

	return fdata.v;
}

void *call(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)data;

	if (argc != 1)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Calling call with wrong number of arguments, expected 1 arguments, got %" PRIuS " arguments", argc);
		return metacall_value_create_int(1);
	}

	if (metacall_value_id(args[0]) != METACALL_STRING)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Calling call with wrong type of arguments, expected %s, got %s",
			metacall_value_id_name(METACALL_STRING), metacall_value_type_name(args[0]));
		return metacall_value_create_int(1);
	}

	/* Parse function call */
	std::string func_str = metacall_value_to_string(args[0]);
	if (func_str.find('(') == std::string::npos || func_str.find(')') == std::string::npos)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "'call' called with mangled function call string: %s", func_str.c_str());
		return metacall_value_create_int(1);
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
		log_write("metacall", LOG_LEVEL_ERROR, "Calling call with wrong number of arguments, expected 2 arguments, got %" PRIuS " arguments", argc);
		return metacall_value_create_int(1);
	}

	if (metacall_value_id(args[0]) != METACALL_STRING && metacall_value_id(args[1]) != METACALL_STRING)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Calling clear with wrong type of arguments, expected two %s, got %s and %s",
			metacall_value_id_name(METACALL_STRING), metacall_value_type_name(args[0]), metacall_value_type_name(args[1]));
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

static std::string formatted_inspect_str;

static void build_inspect_str(const char *str)
{
	formatted_inspect_str += str;
}

void *inspect(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)args;
	(void)data;

	std::mutex formatted_inspect_str_mutex;

	/* Prevent data race when multithreading */
	formatted_inspect_str_mutex.lock();
	formatted_inspect_str = "";

	size_t size = 0;

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };
	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	char *inspect_str = metacall_inspect(&size, allocator);

	void *v = metacall_deserialize(metacall_serial(), inspect_str, size, allocator);

	if (v == NULL)
	{
		std::string err_msg = "Invalid deserialization";

		return metacall_value_create_string(err_msg.c_str(), err_msg.size());
	}

	/* Print run-times */
	value_map_for_each(v, [](const char *key, void *modules) -> void {
		build_inspect_str("runtime ");
		build_inspect_str(key);

		if (metacall_value_count(modules) == 0)
		{
			return;
		}

		build_inspect_str(" {\n");

		/* Print scripts */
		value_array_for_each(modules, [](void *module) -> void {
			/* Get module name */
			void **v_module_map = static_cast<void **>(metacall_value_to_map(module));
			void **v_module_name_tuple = metacall_value_to_array(v_module_map[0]);
			const char *name = metacall_value_to_string(v_module_name_tuple[1]);

			build_inspect_str("\tmodule ");
			build_inspect_str(name);
			build_inspect_str(" { \n");

			/* Get module functions */
			void **v_module_scope_tuple = metacall_value_to_array(v_module_map[1]);
			void **v_scope_map = metacall_value_to_map(v_module_scope_tuple[1]);
			void **v_scope_funcs_tuple = metacall_value_to_array(v_scope_map[1]);

			if (metacall_value_count(v_scope_funcs_tuple[1]) != 0)
			{
				value_array_for_each(v_scope_funcs_tuple[1], [](void *func) -> void {
					/* Get function name */
					void **v_func_map = static_cast<void **>(metacall_value_to_map(func));
					void **v_func_tuple = metacall_value_to_array(v_func_map[0]);
					const char *func_name = metacall_value_to_string(v_func_tuple[1]);

					build_inspect_str("\t\tfunction ");
					build_inspect_str(func_name);
					build_inspect_str("(");

					/* Get function signature */
					void **v_signature_tuple = metacall_value_to_array(v_func_map[1]);
					void **v_args_map = metacall_value_to_map(v_signature_tuple[1]);
					void **v_args_tuple = metacall_value_to_array(v_args_map[1]);
					void **v_args_array = metacall_value_to_array(v_args_tuple[1]);

					size_t iterator = 0, count = metacall_value_count(v_args_tuple[1]);

					value_array_for_each(v_args_array, [&iterator, &count](void *arg) -> void {
						void **v_arg_map = metacall_value_to_map(arg);
						void **v_arg_name_tupla = metacall_value_to_array(v_arg_map[0]);
						std::string parameter_name(metacall_value_to_string(v_arg_name_tupla[1]));

						if (parameter_name.empty())
						{
							parameter_name += "arg";
							parameter_name += std::to_string(iterator);
						}

						build_inspect_str(parameter_name.c_str());

						if (iterator + 1 < count)
						{
							build_inspect_str(", ");
						}

						++iterator;
					});

					build_inspect_str(")\n");
				});
			}

			build_inspect_str("\t}\n");
		});

		build_inspect_str("}\n");
	});

	metacall_value_destroy(v);
	metacall_allocator_free(allocator, inspect_str);
	metacall_allocator_destroy(allocator);

	void *formatted_inspect_str_value = metacall_value_create_string(formatted_inspect_str.c_str(), formatted_inspect_str.size());
	formatted_inspect_str_mutex.unlock();

	return formatted_inspect_str_value;
}

static std::string function_list;

static void build_function_list_str(const char *str)
{
	function_list += str;
}

void *get_function_list(size_t argc, void **argv, void *data)
{
	(void)argc;
	(void)argv;
	(void)data;

	std::mutex function_list_mutex;

	/* Prevent data race when multithreading */
	function_list_mutex.lock();
	function_list = "";

	size_t size = 0;

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };
	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	char *inspect_str = metacall_inspect(&size, allocator);

	void *v = metacall_deserialize(metacall_serial(), inspect_str, size, allocator);

	if (v == NULL)
	{
		std::string err_msg = "Invalid deserialization";

		return metacall_value_create_string(err_msg.c_str(), err_msg.size());
	}

	/* Print run-times */
	value_map_for_each(v, [](const char *key, void *modules) -> void {
		(void)key;

		if (metacall_value_count(modules) == 0)
		{
			return;
		}

		/* Iterate scripts */
		value_array_for_each(modules, [](void *module) -> void {
			/* Get module name */
			void **v_module_map = static_cast<void **>(metacall_value_to_map(module));
			void **v_module_name_tuple = metacall_value_to_array(v_module_map[0]);
			const char *name = metacall_value_to_string(v_module_name_tuple[1]);

			if (std::string(name).find("plugins/") == std::string::npos &&
				std::string(name) != "plugin_extension")
			{
				/* Get module functions */
				void **v_module_scope_tuple = metacall_value_to_array(v_module_map[1]);
				void **v_scope_map = metacall_value_to_map(v_module_scope_tuple[1]);
				void **v_scope_funcs_tuple = metacall_value_to_array(v_scope_map[1]);

				if (metacall_value_count(v_scope_funcs_tuple[1]) != 0)
				{
					value_array_for_each(v_scope_funcs_tuple[1], [](void *func) -> void {
						/* Get function name */
						void **v_func_map = static_cast<void **>(metacall_value_to_map(func));
						void **v_func_tuple = metacall_value_to_array(v_func_map[0]);
						const char *func_name = metacall_value_to_string(v_func_tuple[1]);

						build_function_list_str(func_name);
						build_function_list_str("(");

						/* Get function signature */
						void **v_signature_tuple = metacall_value_to_array(v_func_map[1]);
						void **v_args_map = metacall_value_to_map(v_signature_tuple[1]);
						void **v_args_tuple = metacall_value_to_array(v_args_map[1]);
						void **v_args_array = metacall_value_to_array(v_args_tuple[1]);

						size_t iterator = 0, count = metacall_value_count(v_args_tuple[1]);

						value_array_for_each(v_args_array, [&iterator, &count](void *arg) -> void {
							void **v_arg_map = metacall_value_to_map(arg);
							void **v_arg_name_tupla = metacall_value_to_array(v_arg_map[0]);
							std::string parameter_name(metacall_value_to_string(v_arg_name_tupla[1]));

							if (parameter_name.empty())
							{
								parameter_name += "arg";
								parameter_name += std::to_string(iterator);
							}

							build_function_list_str(parameter_name.c_str());

							if (iterator + 1 < count)
							{
								build_function_list_str(", ");
							}

							++iterator;
						});

						build_function_list_str(")\n");
					});
				}
			}
		});
	});

	metacall_value_destroy(v);
	metacall_allocator_free(allocator, inspect_str);
	metacall_allocator_destroy(allocator);

	void *function_list_value = metacall_value_create_string(function_list.c_str(), function_list.size());
	function_list_mutex.unlock();

	return function_list_value;
}

int cli_core_plugin(void *loader, void *handle, void *context)
{
	(void)handle;
	int ret = 0;

	{
		enum metacall_value_id *arg_types = NULL;
		if (metacall_register_loaderv(loader, context, "get_function_list", get_function_list, METACALL_STRING, 0, arg_types) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to register function: get_function_list");
			ret = 1;
		}
	}

	{
		enum metacall_value_id *arg_types = NULL;
		if (metacall_register_loaderv(loader, context, "help", help, METACALL_STRING, 0, arg_types) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to register function: help");
			ret = 1;
		}
	}

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
