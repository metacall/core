/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

/* -- Headers -- */

#include <metacallcli/application.hpp>

#if defined __has_include
	#if __has_include(<filesystem>)
		#include <filesystem>
namespace fs = std::filesystem;
	#elif __has_include(<experimental/filesystem>)
		#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
	#else
		#error "Missing the <filesystem> header."
	#endif
#else
	#error "C++ standard too old for compiling this file."
#endif

#include <algorithm>
#include <functional>
#include <iostream>

/* -- Namespace Declarations -- */

using namespace metacallcli;

/* -- Private Data -- */

static bool exit_condition = true;

/* -- Methods -- */

void application::repl()
{
	/* Initialize REPL plugins */
	if (!load_path("repl", &plugin_repl_handle))
	{
		/* Do not enter into the main loop */
		exit_condition = true;
		return;
	}

	/* Register exit function */
	auto exit = [](size_t argc, void *args[], void *data) -> void * {
		(void)args;
		(void)data;

		/* Validate function parameters */
		if (argc != 0)
		{
			std::cout << "Invalid number of arguments passed to exit, expected 0, received: " << argc << std::endl;
		}

		std::cout << "Exiting ..." << std::endl;

		/* Exit from main loop */
		exit_condition = true;

		return NULL;
	};

	int result = metacall_register_loaderv(metacall_loader("ext"), plugin_repl_handle, "exit", exit, METACALL_INVALID, 0, NULL);

	if (result != 0)
	{
		std::cout << "Exit function was not registered properly, return code: " << result << std::endl;
	}
	else
	{
		/* Start the main loop */
		exit_condition = false;
	}

	/* Initialize REPL descriptors */
	std::string plugin_path(metacall_plugin_path());

	void *args[] = {
		metacall_value_create_string(plugin_path.c_str(), plugin_path.length())
	};

	void *ret = metacallhv_s(plugin_cli_handle, "repl_initialize", args, sizeof(args) / sizeof(args[0]));

	metacall_value_destroy(args[0]);

	check_for_exception(ret);
}

bool application::cmd(std::vector<std::string> &arguments)
{
	/* Get the command parsing function */
	void *command_parse_func = metacall_handle_function(plugin_cli_handle, "command_parse");

	/* By default, when executing the cmd, it will exit of the REPL */
	exit_condition = true;

	if (command_parse_func == NULL)
	{
		std::cout << "Warning: CLI Arguments Parser was not loaded, "
					 "using fallback argument parser with positional arguments only. "
				  << std::endl
				  << "Any command line option like '--help' will result into error. "
					 "Only files are allowed: $ metacall a.py b.js c.rb"
				  << std::endl;

		/* Use fallback parser, it can execute files but does not support command line arguments as options (i.e: -h, --help) */
		/* Parse program arguments if any (e.g metacall (0) a.py (1) b.js (2) c.rb (3)) */
		arguments_parse(arguments);

		return true;
	}

	/* Check first if the command function is registered */
	void *command_function_func = metacall_handle_function(plugin_cli_handle, "command_function");

	if (command_function_func == NULL)
	{
		return false;
	}

	/* Initialize CMD plugins */
	if (!load_path("cmd", &plugin_cmd_handle))
	{
		return false;
	}

	/* Initialize CMD descriptors */
	std::string plugin_path(metacall_plugin_path());

	void *args[] = {
		metacall_value_create_string(plugin_path.c_str(), plugin_path.length())
	};

	void *command_initialize_ret = metacallhv_s(plugin_cli_handle, "command_initialize", args, sizeof(args) / sizeof(args[0]));

	check_for_exception(command_initialize_ret);

	/* Convert all arguments into metacall value strings */
	std::vector<void *> arguments_values;
	arguments_values.reserve(arguments.size());

	for (const std::string &str_argument : arguments)
	{
		arguments_values.push_back(metacall_value_create_string(str_argument.c_str(), str_argument.length()));
	}

	/* Parse the arguments with the CMD plugin command parse function */
	void *ret = metacallfv_s(command_parse_func, arguments_values.data(), arguments_values.size());

	/* Destroy all the command parse values */
	for (void *value_argument : arguments_values)
	{
		metacall_value_destroy(value_argument);
	}

	/* Check for correct result of command parse */
	if (metacall_value_id(ret) != METACALL_ARRAY)
	{
		check_for_exception(ret);
		return false;
	}

	/* Get the argument map and the positional array */
	void **ret_array = metacall_value_to_array(ret);
	void **command_map = metacall_value_to_map(ret_array[0]);
	size_t command_size = metacall_value_count(ret_array[0]);
	void **positional_array = metacall_value_to_array(ret_array[1]);
	size_t positional_size = metacall_value_count(ret_array[1]);

	/* Execute arguments */
	for (size_t iterator = 0; iterator < command_size; ++iterator)
	{
		void **command_pair = metacall_value_to_array(command_map[iterator]);

		void *args[] = {
			command_pair[0]
		};

		void *command_func = metacallfv_s(command_function_func, args, sizeof(args) / sizeof(args[0]));

		if (metacall_value_id(command_func) == METACALL_FUNCTION)
		{
			/* Execute the function */
			void *command_ret = metacallfv_s(command_func, &command_pair[1], 1);
			metacall_value_destroy(command_func);
			check_for_exception(command_ret);
			continue;
		}
		else if (metacall_value_id(command_func) == METACALL_DOUBLE)
		{
			static const double COMMAND_NOT_REGISTERED = 0.0;

			/* The command is not registered, skip it */
			if (metacall_value_to_double(command_func) == COMMAND_NOT_REGISTERED)
			{
				metacall_value_destroy(command_func);
				continue;
			}

			/* If the function is undefined, try to match the command with a function in the handle scope */
			metacall_value_destroy(command_func);
		}
		else
		{
			check_for_exception(command_func);
			return false;
		}

		/* Otherwise use the cmd handle scope for obtaining the function */
		const char *command_str = metacall_value_to_string(command_pair[0]);
		command_func = metacall_handle_function(plugin_cmd_handle, command_str);

		if (command_func == NULL)
		{
			std::cout << "Error: Command line option '" << command_str << "' unrecognized" << std::endl;
			return false;
		}

		void *command_ret = metacallfv_s(command_func, &command_pair[1], 1);

		check_for_exception(command_ret);
	}

	/* Note: If it has zero positional arguments, we should also run the repl, for example:
	*  $ metacall --some-option --another-option
	*/
	if (positional_size == 0)
	{
		/* Initialize the REPL */
		repl();
		exit_condition = false;
	}
	else
	{
		/* Execute the positional arguments */
		std::vector<std::string> positional_arguments;
		positional_arguments.reserve(positional_size);

		for (size_t iterator = 0; iterator < positional_size; ++iterator)
		{
			positional_arguments.push_back(metacall_value_to_string(positional_array[iterator]));
		}

		arguments_parse(positional_arguments);
	}

	metacall_value_destroy(ret);

	return true;
}

application::application(int argc, char *argv[]) :
	plugin_cli_handle(NULL), plugin_repl_handle(NULL), plugin_cmd_handle(NULL)
{
	/* Initialize MetaCall */
	if (metacall_initialize() != 0)
	{
		/* Exit from application */
		return;
	}

	/* Initialize MetaCall arguments */
	metacall_initialize_args(argc, argv);

	/* Print MetaCall information */
	metacall_print_info();

	/* Initialize CLI internal plugins */
	if (!load_path("internal", &plugin_cli_handle))
	{
		/* Do not enter into the main loop */
		exit_condition = true;
		return;
	}

	if (argc == 1)
	{
		/* Launch the REPL */
		repl();
	}
	else
	{
		std::vector<std::string> arguments(argv + 1, argv + argc);

		/* Launch the CMD (parse arguments) */
		if (!cmd(arguments))
		{
			/* TODO: Report something? */
		}
	}
}

application::~application()
{
	metacall_destroy();
}

void application::arguments_parse(std::vector<std::string> &arguments)
{
	/* List of file extensions mapped into loader tags */
	static std::unordered_map<std::string, std::string> extension_to_tag = {
		/* Mock Loader */
		{ "mock", "mock" },
		/* Python Loader */
		{ "py", "py" },
		/* Ruby Loader */
		{ "rb", "rb" },
		/* C# Loader */
		{ "cs", "cs" },
		{ "dll", "cs" },
		{ "vb", "cs" },
		/* Cobol Loader */
		{ "cob", "cob" },
		{ "cbl", "cob" },
		{ "cpy", "cob" },
		/* NodeJS Loader */
		{ "js", "node" },
		{ "node", "node" },
		/* TypeScript Loader */
		{ "ts", "ts" },
		{ "jsx", "ts" },
		{ "tsx", "ts" },
		/* WASM Loader */
		{ "wasm", "wasm" },
		{ "wat", "wasm" },
		/* Rust Loader */
		{ "rs", "rs" },
		/* C Loader */
		{ "c", "c" },
		{ "h", "c" },
		/* Java Loader */
		{ "java", "java" },
		{ "jar", "java" },
		/* RPC Loader */
		{ "rpc", "rpc" }

		// TODO: Implement handling of duplicated extensions,
		// load the file with all loaders of each duplicated extension (trial and error)

		// /* Extension Loader */
		// { "so", "ext" },
		// { "dylib", "ext" },
		// { "dll", "ext" },

		/* Note: By default js extension uses NodeJS loader instead of JavaScript V8 */
		/* Probably in the future we can differenciate between them, but it is not trivial */
	};

	for (std::string script : arguments)
	{
		const std::string tag = extension_to_tag[script.substr(script.find_last_of(".") + 1)];
		const std::string safe_tag = tag != "" ? tag : "file"; /* Use File Loader if the tag is not found */

		/* Load the script */
		const char *scripts[1] = {
			script.c_str()
		};

		if (metacall_load_from_file(safe_tag.c_str(), scripts, 1, NULL) != 0)
		{
			/* Stop loading more scripts */
			std::cout << "Error: Failed to load script '" << script << "' with loader '" << safe_tag << "'" << std::endl;
			return;
		}
	}
}

bool application::load_path(const char *path, void **handle)
{
	/* Get core plugin path and handle in order to load cli plugins */
	const char *plugin_path = metacall_plugin_path();
	void *plugin_extension_handle = metacall_plugin_extension();

	if (plugin_path == NULL || plugin_extension_handle == NULL)
	{
		return false;
	}

	/* Define the cli plugin path as string (core plugin path plus the subpath) */
	fs::path plugin_cli_path(plugin_path);
	plugin_cli_path /= "cli";
	plugin_cli_path /= path;
	std::string plugin_cli_path_str(plugin_cli_path.string());

	/* Load cli plugins into plugin cli handle */
	void *args[] = {
		metacall_value_create_string(plugin_cli_path_str.c_str(), plugin_cli_path_str.length()),
		metacall_value_create_ptr(handle)
	};

	void *ret = metacallhv_s(plugin_extension_handle, "plugin_load_from_path", args, sizeof(args) / sizeof(args[0]));

	metacall_value_destroy(args[0]);
	metacall_value_destroy(args[1]);

	if (ret == NULL)
	{
		std::cout << "Failed to load CLI plugins from folder: " << plugin_cli_path_str << std::endl;
		return false;
	}

	if (metacall_value_id(ret) == METACALL_INT && metacall_value_to_int(ret) != 0)
	{
		std::cout << "Failed to load CLI plugins from folder '"
				  << plugin_cli_path_str << "' with result: "
				  << metacall_value_to_int(ret) << std::endl;

		metacall_value_destroy(ret);
		return false;
	}

	metacall_value_destroy(ret);

	return true;
}

void application::run()
{
	void *evaluate_func = metacall_handle_function(plugin_cli_handle, "repl_evaluate");

	while (exit_condition != true)
	{
		std::mutex await_mutex;
		std::condition_variable await_cond;
		std::unique_lock<std::mutex> lock(await_mutex);

		struct await_data_type
		{
			void *v;
			std::mutex &mutex;
			std::condition_variable &cond;
			bool exit_condition;
		};

		struct await_data_type await_data = { NULL, await_mutex, await_cond, false };

		void *future = metacallfv_await_s(
			evaluate_func, metacall_null_args, 0,
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
				await_data->exit_condition = true;
				await_data->cond.notify_one();
				return NULL;
			},
			static_cast<void *>(&await_data));

		await_cond.wait(lock);

		/* Unused */
		metacall_value_destroy(future);

		/* Check if the loop was rejected */
		if (await_data.exit_condition)
		{
			exit_condition = true;
		}
		else
		{
			void **results = metacall_value_to_array(await_data.v);
			void *args[2];

			if (metacall_value_id(results[0]) == METACALL_EXCEPTION || metacall_value_id(results[0]) == METACALL_THROWABLE)
			{
				args[0] = metacall_value_copy(results[0]);
				args[1] = metacall_value_create_null();
			}
			else
			{
				args[0] = metacall_value_create_null();

				/* Execute command */
				if (metacall_value_id(results[0]) == METACALL_ARRAY)
				{
					args[1] = execute(results[0]);
				}
				else
				{
					args[1] = metacall_value_copy(results[0]);
				}

				if (metacall_value_id(args[1]) == METACALL_INVALID)
				{
					metacall_value_destroy(args[1]);
					args[1] = metacall_value_create_null();
				}
			}

			/* Invoke next iteration of the REPL */
			void *ret = metacallfv_s(metacall_value_to_function(results[1]), args, 2);

			check_for_exception(ret);

			metacall_value_destroy(args[0]);
			metacall_value_destroy(args[1]);
		}

		metacall_value_destroy(await_data.v);
	}

	if (plugin_cli_handle != NULL)
	{
		/* Close REPL */
		void *ret = metacallhv_s(plugin_cli_handle, "repl_close", metacall_null_args, 0);

		check_for_exception(ret);

		/* Get the command destroy function */
		void *command_destroy_func = metacall_handle_function(plugin_cli_handle, "command_destroy");

		/* Destroy the commands */
		if (command_destroy_func != NULL)
		{
			/* Parse the arguments with the CMD plugin command parse function */
			ret = metacallfv_s(command_destroy_func, metacall_null_args, 0);

			check_for_exception(ret);
		}
	}
}

void *application::execute(void *tokens)
{
	size_t size = metacall_value_count(tokens);

	if (size == 0)
	{
		return metacall_value_create_null();
	}
	else
	{
		void **tokens_array = metacall_value_to_array(tokens);
		void *key = tokens_array[0];

		return metacallhv_s(plugin_repl_handle, metacall_value_to_string(key), &tokens_array[1], size - 1);
	}
}

void application::check_for_exception(void *v)
{
	struct metacall_exception_type ex;

	if (metacall_error_from_value(v, &ex) == 0)
	{
		std::cout << "Exception: " << ex.message << std::endl
				  << ex.stacktrace << std::endl;
	}

	metacall_value_destroy(v);
}
