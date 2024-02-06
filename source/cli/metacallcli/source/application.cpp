/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

/* -- Headers -- */

#include <metacallcli/application.hpp>
#include <metacallcli/parser.hpp>

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

static bool exit_condition = true;

/* -- Methods -- */

application::parameter_iterator::parameter_iterator(application &app) :
	app(app)
{
}

application::parameter_iterator::~parameter_iterator()
{
}

void application::parameter_iterator::operator()(const char *parameter)
{
	std::string script(parameter);

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

		// TODO: Implement handling of duplicated extensions, load the file with all loaders (trial and error)

		// /* Extension Loader */
		// { "so", "ext" },
		// { "dylib", "ext" },
		// { "dll", "ext" },

		/* Note: By default js extension uses NodeJS loader instead of JavaScript V8 */
		/* Probably in the future we can differenciate between them, but it is not trivial */
	};

	const std::string tag = extension_to_tag[script.substr(script.find_last_of(".") + 1)];
	const std::string safe_tag = tag != "" ? tag : "file"; /* Use File Loader if the tag is not found */

	/* Load the script */
	void *args[2] = {
		metacall_value_create_string(safe_tag.c_str(), safe_tag.length()),
		metacall_value_create_string(script.c_str(), script.length())
	};

	app.invoke("load", args, 2);
	app.shutdown();
}

application::application(int argc, char *argv[]) :
	plugin_cli_handle(NULL), plugin_repl_handle(NULL)
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

	// TODO: Implement a new plugin for parsing command line options

	/* TODO: This has been updated, review it: */
	/* Parse program arguments if any (e.g metacall (0) a.py (1) b.js (2) c.rb (3)) */
	if (argc > 1)
	{
		parameter_iterator param_it(*this);

		/* TODO: This has been refactored in order to pass the arguments to the runtimes */
		/* Using argv + 2 by now, but this should be deleted in a near future or review the implementation */

		/* Parse program parameters */
		std::for_each(&argv[1], argv + /*argc*/ 2, param_it);

		return;
	}

	/* Initialize REPL plugins */
	if (!load_path("repl", &plugin_repl_handle))
	{
		/* Do not enter into the main loop */
		exit_condition = true;
		plugin_repl_handle = NULL;
		return;
	}

	/* Initialize CLI plugins */
	if (load_path("cli", &plugin_cli_handle))
	{
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

		int result = metacall_register_loaderv(metacall_loader("ext"), plugin_cli_handle, "exit", exit, METACALL_INVALID, 0, NULL);

		if (result != 0)
		{
			std::cout << "Exit function was not registered properly, return code: " << result << std::endl;
		}
		else
		{
			/* Start the main loop */
			exit_condition = false;
		}
	}
}

application::~application()
{
	int result = metacall_destroy();

	if (result != 0)
	{
		std::cout << "Error while destroying MetaCall, exit code: " << result << std::endl;
	}
}

bool application::load_path(const char *path, void **handle)
{
	/* Get core plugin path and handle in order to load cli plugins */
	const char *plugin_path = metacall_plugin_path();
	void *plugin_extension_handle = metacall_plugin_extension();

	if (plugin_path == NULL || plugin_extension_handle == NULL)
	{
		return NULL;
	}

	/* Define the cli plugin path as string (core plugin path plus the subpath) */
	fs::path plugin_cli_path(plugin_path);
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
	void *evaluate_func = metacall_handle_function(plugin_repl_handle, "evaluate");

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
			exit_condition = await_data.exit_condition;
		}
		else
		{
			void **results = metacall_value_to_array(await_data.v);

			/* Execute command */
			void *command_result = execute(results[0]);
			void *args[2];

			args[0] = metacall_value_create_null();

			if (metacall_value_id(command_result) == METACALL_THROWABLE)
			{
				args[1] = metacall_throwable_value(metacall_value_to_throwable(command_result));
			}
			else
			{
				args[1] = command_result;
			}

			/* Invoke next iteration of the REPL */
			void *ret = metacallfv_s(metacall_value_to_function(results[1]), args, 2);

			metacall_value_destroy(args[0]);
			metacall_value_destroy(ret);
			metacall_value_destroy(command_result);
		}

		metacall_value_destroy(await_data.v);
	}

	/* Close REPL */
	if (plugin_repl_handle != NULL)
	{
		void *ret = metacallhv_s(plugin_repl_handle, "close", metacall_null_args, 0);

		metacall_value_destroy(ret);
	}
}

void application::shutdown()
{
	exit_condition = true;
}

void application::print(void *v)
{
	if (v == NULL)
	{
		std::cout << "null" << std::endl;
	}
	else
	{
		if (metacall_value_id(v) == METACALL_THROWABLE)
		{
			// TODO: Implement print for throwable
			std::cout << "TODO: Throwable" << std::endl;
		}
		else
		{
			size_t size = 0;
			struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };
			void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);
			char *value_str = metacall_serialize(metacall_serial(), v, &size, allocator);

			std::cout << value_str << std::endl;

			metacall_allocator_free(allocator, value_str);
		}

		metacall_value_destroy(v);
	}
}

void application::invoke(const char *func, void *args[], size_t size)
{
	void *ret = metacallhv_s(plugin_cli_handle, func, args, size);

	if (metacall_value_id(ret) == METACALL_INT)
	{
		int result = metacall_value_to_int(ret);

		if (result != 0)
		{
			std::cout << "Failed to execute '" << func << "' command, return code: " << result;
		}

		metacall_value_destroy(ret);
	}
	else
	{
		print(ret);
	}

	for (size_t it = 0; it < size; ++it)
	{
		metacall_value_destroy(args[it]);
	}
}

void *application::execute(void *tokens)
{
	size_t size = metacall_value_count(tokens);
	void **tokens_array = metacall_value_to_array(tokens);
	void *key = tokens_array[0];

	return metacallhv_s(plugin_cli_handle, metacall_value_to_string(key), &tokens_array[1], size - 1);
}

void *application::argument_parse(parser_parameter &p)
{
	if (p.is<bool>())
	{
		bool b = p.to<bool>();

		boolean bo = static_cast<boolean>(b);

		return metacall_value_create_bool(bo);
	}
	else if (p.is<char>())
	{
		char c = p.to<char>();

		return metacall_value_create_char(c);
	}
	else if (p.is<int>())
	{
		int i = p.to<int>();

		return metacall_value_create_int(i);
	}
	else if (p.is<long>())
	{
		long l = p.to<long>();

		return metacall_value_create_long(l);
	}
	else if (p.is<float>())
	{
		float f = p.to<float>();

		return metacall_value_create_float(f);
	}
	else if (p.is<double>())
	{
		double d = p.to<double>();

		return metacall_value_create_double(d);
	}
	else if (p.is<void *>())
	{
		void *ptr = p.to<void *>();

		return metacall_value_create_ptr(ptr);
	}
	else if (p.is<std::string>())
	{
		std::string str = p.to<std::string>();

		return metacall_value_create_string(str.c_str(), str.length());
	}

	return NULL;
}
