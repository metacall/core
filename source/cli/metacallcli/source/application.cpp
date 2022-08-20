/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

/* -- Headers -- */

#include <metacallcli/application.hpp>

#include <string.h>

#include <algorithm>
#include <filesystem>
#include <functional>
#include <iostream>

/* TODO: Windows special characters not working properly */
/* Set UTF-16 mode for stdout in Windows for the lambda character */
/*
#if defined(WIN32) || defined(_WIN32)
#	include <windows.h>
#	include <io.h>
#	include <fcntl.h>
#endif
*/

#include <clocale>

/* -- Namespace Declarations -- */

using namespace metacallcli;

std::unordered_map<std::string, void *> application::function_map;

const char *metacall_usage_str =
	("Usage: metacallcli [files]\n"
	 "       metacallcli [cmd] [arguments]\n\n"

	 "MetaCall is a extensible, embeddable and interoperable cross-platform\n"
	 "polyglot runtime. It supports NodeJS, Vanilla JavaScript, TypeScript, \n"
	 "Python, Ruby, C#, Java, WASM, Go, C, C++, Rust, D, Cobol and more.\n"
	 "MetaCall allows calling functions, methods or procedures between \n"
	 "multiple programming languages.\n"

	 "options\n\n"
	 "--help            Display this information\n"
	 "--version         Display metacall runtime version information\n"

	 "commands\n"
	 "eval              Evaluate a code snippet with the specified runtime tag: <tag> <code>\n"
	 "                  loader tag: node, py, c...\n"
	 "load              Load a script from file into MetaCall\n"
	 "call              Call a function previously loaded in MetaCall\n"
	 "inspect           Show all runtimes, modules and functions (with their signature)\n"
	 "                  loaded into MetaCall\n"

	 "await             Await an async function previously loaded in MetaCall\n"
	 "clear             Delete a script previously loaded in MetaCall\n"

	 "\n"

	 "'metacall help a' for list of available commands\n"
	 "'metacallcli help <command>' or 'metacallcli <command> --help' to read"
	 "about a specific command\n"
	 "\n"
	 "Documentation can be found here https://github.com/metacall/core\n");

bool application::load(const std::string script)
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
		/* Rust Loader */
		{ "rs", "rs" },
		/*C loader*/
		{ "c", "c" }

		/* Note: By default js extension uses NodeJS loader instead of JavaScript V8 */
		/* Probably in the future we can differenciate between them, but it is not trivial */
	};

	const std::string tag = extension_to_tag[script.substr(script.find_last_of(".") + 1)];
	const std::string safeTag = tag != "" ? tag : "file"; /* Use File Loader if the tag is not found */

	const char *load_scripts[] = {
		script.c_str()
	};

	if (metacall_load_from_file(safeTag.c_str(), load_scripts, sizeof(load_scripts) / sizeof(load_scripts[0]), NULL) != 0)
	{
		std::cerr << "Script (" << script << ") load error in loader (" << tag << ")" << std::endl;

		return false;
	}

	scripts.push_back(script);

	std::cout << "Script (" << script << ") loaded correctly" << std::endl;

	return true;
}

std::string trim(std::string s)
{
	//erase leading and trailing spaces
	auto start = s.begin();
	while (start != s.end() && isspace(*start))
	{
		start++;
	}

	auto end = s.end();
	do
	{
		end--;
	} while (std::distance(start, end) > 0 && isspace(*end));

	return std::string(start, end + 1);
}

size_t tokenize(std::string s, std::string &tok, char delimiter)
{
	s = trim(s);
	std::string::size_type pos;
	if (s[0] == delimiter)
	{
		s.erase(0, 1);
	}

	pos = s.find_first_of(delimiter);
	tok = s.substr(0, pos);
	return pos;
}

static void *execute_cb(size_t argc, void **argv, void *data)
{
	if (argc != 1)
	{
		/* TODO: should probably exit here? */
		std::cerr << "Repl execute_cb called with wrong number of arguments" << '\n';
		return metacall_value_create_int(1);
	}

	(void)data;

	std::string cmdline = trim(metacall_value_to_string(argv[0]));

	/* Get command*/
	std::string cmd = trim(cmdline.substr(0, cmdline.find_first_of("\x20\x09\x0a\x0b")));
	if (application::function_map.find(cmd.c_str()) == application::function_map.end())
	{
		std::string err_msg = "Invalid command: " + cmd;
		return metacall_value_create_string(err_msg.c_str(), err_msg.size());
	}
	else if (cmd == "repl_main")
	{
		std::string err_msg = "Cannot invoke repl as a command";
		return metacall_value_create_string(err_msg.c_str(), err_msg.size());
	}

	/* Parse command args*/
	std::string arg_str;
	std::string::size_type idx = cmdline.find_first_of("\x20\x09\x0a\x0b");
	if (idx != std::string::npos)
		arg_str = trim(cmdline.substr(idx));

	std::vector<std::string> args_list;
	std::string::size_type pos = 0;
	while (pos != std::string::npos)
	{
		std::string token;
		arg_str = trim(arg_str);
		pos = (arg_str[0] == '"') ?
					tokenize(arg_str, token, '\x22') :
					((arg_str[0] == '\x27') ?
							tokenize(arg_str, token, '\x27') :
							tokenize(arg_str, token, '\x20'));

		if (!token.empty())
		{
			args_list.push_back(token);
		}
		arg_str.erase(0, pos + 1);
	}

	/* Verify correct no of args */
	if (args_list.size() < metacall_function_size(application::function_map[cmd]))
	{
		std::string err_msg = "'" + cmd + "' called with too few arguments";
		return metacall_value_create_string(err_msg.c_str(), err_msg.size());
	}

	/* Convert args to array of values*/
	void **vargs = (void **)malloc(sizeof(void *) * args_list.size());
	for (size_t i = 0; i < args_list.size(); i++)
	{
		vargs[i] = metacall_value_create_string(args_list[i].c_str(), args_list[i].size());
	}

	/* Invoke command */
	void *result = metacallfv_s(application::function_map[cmd], vargs, args_list.size());
	if (result == NULL)
	{
		std::string err_msg = "Unable to invoke command. Call to '" + cmd + "' failed\n(null)";
		result = metacall_value_create_string(err_msg.c_str(), err_msg.size());
	}

	/* Destroy value args*/
	for (size_t i = 0; i < args_list.size(); i++)
	{
		metacall_value_destroy(vargs[i]);
	}

	free(vargs);

	return result;
}

application::application(int argc, char *argv[]) :
	exit_condition(false), plugin_cli_handle(NULL)
{
	/* Set locale */
	setlocale(LC_CTYPE, "C");

/* TODO: Windows special characters not working properly */
#if 0
	#if defined(WIN32) || defined(_WIN32)
	{
		// SetConsoleOutputCP(CP_UTF16);

		/* Set UTF-16 mode for stdout in Windows for the lambda character */
		_setmode(_fileno(stdout), _O_U16TEXT);

		/* Enable buffering to prevent VS from chopping up UTF-16 byte sequences */
		setvbuf(stdout, nullptr, _IOFBF, 1000);
	}
	#endif
#endif

	/* Initialize MetaCall */
	if (metacall_initialize() != 0)
	{
		/* Exit from application */
		shutdown();
	}

	/* Initialize MetaCall arguments */
	metacall_initialize_args(argc, argv);

	/* Print MetaCall information */
	metacall_print_info();

	/* Get core plugin path and handle in order to load cli plugins */
	const char *plugin_path = metacall_plugin_path();
	void *plugin_extension_handle = metacall_plugin_extension();

	if (plugin_path != NULL && plugin_extension_handle != NULL)
	{
		/* Define the cli plugin path as string (core plugin path plus cli) */
		namespace fs = std::filesystem;
		fs::path plugin_cli_path(plugin_path);
		plugin_cli_path /= "cli";
		std::string plugin_cli_path_str(plugin_cli_path.string());

		/* Load cli plugins into plugin cli handle */
		void *args[] = {
			metacall_value_create_string(plugin_cli_path_str.c_str(), plugin_cli_path_str.length()),
			metacall_value_create_ptr(&plugin_cli_handle)
		};

		void *ret = metacallhv_s(plugin_extension_handle, "plugin_load_from_path", args, sizeof(args) / sizeof(args[0]));

		if (ret == NULL || (ret != NULL && metacall_value_to_int(ret) != 0))
		{
			std::cerr << "Failed to load CLI plugins from folder: " << plugin_cli_path_str << std::endl;
		}

		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);
		metacall_value_destroy(ret);
	}
}

application::~application()
{
	if (metacall_destroy() != 0)
	{
		std::cerr << "Error while destroying MetaCall." << std::endl;
	}
}

void application::run()
{
	int argc = metacall_argc();
	char **argv = metacall_argv();

	void ***handle_export = (void ***)metacall_handle_export(plugin_cli_handle);
	size_t handle_size = metacall_value_count(handle_export);

	void **commands = (void **)malloc(sizeof(void *) * handle_size);

	/* Build command function map*/
	for (size_t i = 0; i < handle_size; i++)
	{
		function_map[metacall_value_to_string(handle_export[i][0])] = metacall_value_to_function(handle_export[i][1]);
		commands[i] = handle_export[i][0];
	}

	if (argc == 1)
	{
		/* Register repl callback function */
		void *func_execute_cb = NULL;
		enum metacall_value_id arg_types[] = { METACALL_STRING };

		if (metacall_registerv(NULL, &execute_cb, &func_execute_cb, METACALL_STRING, 1, arg_types) != 0 ||
			func_execute_cb == NULL)
		{
			std::cerr << "Failed to register repl callback function" << '\n';
			shutdown(1);
		}

		/* Build repl value args */
		void *args[] = {
			metacall_value_create_function(func_execute_cb),
			metacall_value_create_function(function_map["get_function_list"]),
			metacall_value_create_array(const_cast<const void **>(commands), handle_size)
		};

		/* Invoke repl*/
		void *ret = metacallfv_s(function_map["repl_main"], args, sizeof(args) / sizeof(args[0]));

		if (ret == NULL)
		{
			shutdown(1, "Unable to start repl");
		}

		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);
		//metacall_value_destroy(args[2]); /* args[2] is free'd in line 437 */
		metacall_value_destroy(ret);
	}
	else
	{
		argv++;
		if (!strcmp(*argv, "--help") ||
			!strcmp(*argv, "-h"))
		{
			std::cout << metacall_usage_str << '\n'
					  << metacall_print_info();
		}
		else if (!strcmp(*argv, "--version") ||
				 !strcmp(*argv, "version"))
		{
			std::cout << metacall_print_info() << '\n';
		}
		else
		{
			/*Execute argument if it's a command*/
			if (function_map.find(*argv) != function_map.end())
			{
				std::string cmdline = *argv++;
				argc = argc - 2;

				while (argc > 0)
				{
					cmdline += ' ';
					cmdline += *argv++;
					argc--;
				}

				void *args[] = {
					metacall_value_create_string(cmdline.c_str(), cmdline.size())
				};

				void *result = execute_cb(1, args, NULL);

				if (result != NULL)
				{
					char *result_str = NULL;
					if (metacall_value_id(result) == METACALL_STRING)
					{
						result_str = metacall_value_to_string(result);
					}
					else
					{
						/* Else serialize result*/
						struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

						void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

						size_t size = 0;

						result_str = metacall_serialize(metacall_serial(), result, &size, allocator);
						metacall_allocator_destroy(allocator);
					}
					metacall_value_destroy(result);

					std::cout << result_str << '\n';
				}
			}

			/*Load all files*/
			for (; argc > 1; argc--)
			{
				if (!load(*argv++))
				{
					shutdown(1);
				}
			}
		}
	}

	metacall_value_destroy(handle_export);
	free(commands);
}

void application::shutdown(int exitcode, std::string message)
{
	if (exitcode)
	{
		std::cerr << message << '\n';
		exit(exitcode);
	}
	else
	{
		std::cout << message << '\n';
		exit(exitcode);
	}
}
