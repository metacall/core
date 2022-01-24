/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

/* -- Headers -- */

#include <metacallcli/application.hpp>
#include <metacallcli/parser.hpp>
#include <metacallcli/tokenizer.hpp>

#include <algorithm>
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

/* -- Private Methods -- */

bool command_cb_help(application & /*app*/, tokenizer & /*t*/)
{
	std::cout << "MetaCall Command Line Interface by Parra Studios" << std::endl;
	std::cout << "Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>" << std::endl;
	std::cout << std::endl
			  << "A command line interface for MetaCall Core" << std::endl;

	/* Command list */
	std::cout << std::endl
			  << "Command list:" << std::endl
			  << std::endl;

	/* Load command */
	std::cout << "\t┌────────────────────────────────────────────────────────────────────────────────────────┐" << std::endl;
	std::cout << "\t│ Load a script from file into MetaCall                                                  │" << std::endl;
	std::cout << "\t│────────────────────────────────────────────────────────────────────────────────────────│" << std::endl;
	std::cout << "\t│ Usage:                                                                                 │" << std::endl;
	std::cout << "\t│ load <runtime tag> <script0> <script1> ... <scriptN>                                   │" << std::endl;
	std::cout << "\t│    <runtime tag> : identifier to the type of script                                    │" << std::endl;
	std::cout << "\t│                  options :                                                             │" << std::endl;
	std::cout << "\t│                            mock - Mock (for testing purposes)                          │" << std::endl;
	std::cout << "\t│                            py   - Python                                               │" << std::endl;
	std::cout << "\t│                            node - NodeJS                                               │" << std::endl;
	std::cout << "\t│                            rb   - Ruby                                                 │" << std::endl;
	std::cout << "\t│                            cs   - C# NetCore                                           │" << std::endl;
	std::cout << "\t│                            cob  - Cobol                                                │" << std::endl;
	std::cout << "\t│                            ts   - TypeScript                                           │" << std::endl;
	std::cout << "\t│                            js   - V8 JavaScript Engine                                 │" << std::endl;
	std::cout << "\t│                            file - Files (for handling file systems)                    │" << std::endl;
	std::cout << "\t│    <script0> <script1> ... <scriptN> : relative or absolute path to the script(s)      │" << std::endl;
	std::cout << "\t│                                                                                        │" << std::endl;
	std::cout << "\t│ Example:                                                                               │" << std::endl;
	std::cout << "\t│ load node concat.js                                                                    │" << std::endl;
	std::cout << "\t│ load py example.py                                                                     │" << std::endl;
	std::cout << "\t│ load rb hello.rb                                                                       │" << std::endl;
	std::cout << "\t│                                                                                        │" << std::endl;
	std::cout << "\t│ Result:                                                                                │" << std::endl;
	std::cout << "\t│ Script (concat.js) loaded correctly                                                    │" << std::endl;
	std::cout << "\t└────────────────────────────────────────────────────────────────────────────────────────┘" << std::endl
			  << std::endl;

	/* Inspect command */
	std::cout << "\t┌────────────────────────────────────────────────────────────────────────────────────────┐" << std::endl;
	std::cout << "\t│ Show all runtimes, modules and functions (with their signature) loaded into MetaCall   │" << std::endl;
	std::cout << "\t│────────────────────────────────────────────────────────────────────────────────────────│" << std::endl;
	std::cout << "\t│ Usage:                                                                                 │" << std::endl;
	std::cout << "\t│ inspect                                                                                │" << std::endl;
	std::cout << "\t│                                                                                        │" << std::endl;
	std::cout << "\t│ Example:                                                                               │" << std::endl;
	std::cout << "\t│ inspect                                                                                │" << std::endl;
	std::cout << "\t│                                                                                        │" << std::endl;
	std::cout << "\t│ Result:                                                                                │" << std::endl;
	std::cout << "\t│ runtime node {                                                                         │" << std::endl;
	std::cout << "\t│     module concat {                                                                    │" << std::endl;
	std::cout << "\t│         function concat(left, right)                                                   │" << std::endl;
	std::cout << "\t│     }                                                                                  │" << std::endl;
	std::cout << "\t│ }                                                                                      │" << std::endl;
	std::cout << "\t└────────────────────────────────────────────────────────────────────────────────────────┘" << std::endl
			  << std::endl;

	/* Eval command */
	std::cout << "\t┌────────────────────────────────────────────────────────────────────────────────────────┐" << std::endl;
	std::cout << "\t│ Evaluate a code snippet with the specified runtime tag                                 │" << std::endl;
	std::cout << "\t│────────────────────────────────────────────────────────────────────────────────────────│" << std::endl;
	std::cout << "\t│ Usage:                                                                                 │" << std::endl;
	std::cout << "\t│ eval <runtime tag> <script>                                                            │" << std::endl;
	std::cout << "\t│    <runtime tag> : identifier to the type of script                                    │" << std::endl;
	std::cout << "\t│                  options :                                                             │" << std::endl;
	std::cout << "\t│                            mock - Mock (for testing purposes)                          │" << std::endl;
	std::cout << "\t│                            py   - Python                                               │" << std::endl;
	std::cout << "\t│                            node - NodeJS                                               │" << std::endl;
	std::cout << "\t│                            rb   - Ruby                                                 │" << std::endl;
	std::cout << "\t│                            cs   - C# NetCore                                           │" << std::endl;
	std::cout << "\t│                            cob  - Cobol                                                │" << std::endl;
	std::cout << "\t│                            ts   - TypeScript                                           │" << std::endl;
	std::cout << "\t│                            js   - V8 JavaScript Engine                                 │" << std::endl;
	std::cout << "\t│                            file - Files (for handling file systems)                    │" << std::endl;
	std::cout << "\t│    <script> : textual code to execute                                                  │" << std::endl;
	std::cout << "\t│                                                                                        │" << std::endl;
	std::cout << "\t│ Example:                                                                               │" << std::endl;
	std::cout << "\t│ eval node console.log(\"hello world\")                                                   │" << std::endl;
	std::cout << "\t│                                                                                        │" << std::endl;
	std::cout << "\t│ Result:                                                                                │" << std::endl;
	std::cout << "\t│ \"hello world\"                                                                          │" << std::endl;
	std::cout << "\t└────────────────────────────────────────────────────────────────────────────────────────┘" << std::endl
			  << std::endl;

	/* Call command */
	std::cout << "\t┌────────────────────────────────────────────────────────────────────────────────────────┐" << std::endl;
	std::cout << "\t│ Call a function previously loaded in MetaCall                                          │" << std::endl;
	std::cout << "\t│────────────────────────────────────────────────────────────────────────────────────────│" << std::endl;
	std::cout << "\t│ Usage:                                                                                 │" << std::endl;
	std::cout << "\t│ call <function name>(<arg0>, <arg1>, ... , <argN>)                                     │" << std::endl;
	std::cout << "\t│    <function name> : alphanumeric string beginning by letter (method1, method2, hello) │" << std::endl;
	std::cout << "\t│    <arg0>, <arg1>, ... , <argN>  : arguments to be passed to the function in JSON      │" << std::endl;
	std::cout << "\t│                                    types :                                             │" << std::endl;
	std::cout << "\t│                                            bool   - true, false                        │" << std::endl;
	std::cout << "\t│                                            number - 5, 4.34                            │" << std::endl;
	std::cout << "\t│                                            string - \"hello world\"                      │" << std::endl;
	std::cout << "\t│                                            array  - [2, true, \"abc\"]                   │" << std::endl;
	std::cout << "\t│                                            object - { \"one\": 1, \"two\": 2 }             │" << std::endl;
	std::cout << "\t│                                                                                        │" << std::endl;
	std::cout << "\t│ Example:                                                                               │" << std::endl;
	std::cout << "\t│ call concat(\"hello\", \"world\")                                                          │" << std::endl;
	std::cout << "\t│                                                                                        │" << std::endl;
	std::cout << "\t│ Result:                                                                                │" << std::endl;
	std::cout << "\t│ \"hello world\"                                                                          │" << std::endl;
	std::cout << "\t└────────────────────────────────────────────────────────────────────────────────────────┘" << std::endl
			  << std::endl;

	/* Await command */
	std::cout << "\t┌────────────────────────────────────────────────────────────────────────────────────────┐" << std::endl;
	std::cout << "\t│ Await an async function previously loaded in MetaCall                                  │" << std::endl;
	std::cout << "\t│────────────────────────────────────────────────────────────────────────────────────────│" << std::endl;
	std::cout << "\t│ Usage:                                                                                 │" << std::endl;
	std::cout << "\t│ await <function name>(<arg0>, <arg1>, ... , <argN>)                                    │" << std::endl;
	std::cout << "\t│    <function name> : alphanumeric string beginning by letter (method1, method2, hello) │" << std::endl;
	std::cout << "\t│    <arg0>, <arg1>, ... , <argN>  : arguments to be passed to the function in JSON      │" << std::endl;
	std::cout << "\t│                                    types :                                             │" << std::endl;
	std::cout << "\t│                                            bool   - true, false                        │" << std::endl;
	std::cout << "\t│                                            number - 5, 4.34                            │" << std::endl;
	std::cout << "\t│                                            string - \"hello world\"                      │" << std::endl;
	std::cout << "\t│                                            array  - [2, true, \"abc\"]                   │" << std::endl;
	std::cout << "\t│                                            object - { \"one\": 1, \"two\": 2 }             │" << std::endl;
	std::cout << "\t│                                                                                        │" << std::endl;
	std::cout << "\t│ Example:                                                                               │" << std::endl;
	std::cout << "\t│ await concat(\"hello\", \"world\")                                                         │" << std::endl;
	std::cout << "\t│                                                                                        │" << std::endl;
	std::cout << "\t│ Result:                                                                                │" << std::endl;
	std::cout << "\t│ \"hello world\"                                                                          │" << std::endl;
	std::cout << "\t└────────────────────────────────────────────────────────────────────────────────────────┘" << std::endl
			  << std::endl;

	/* Clear command */
	std::cout << "\t┌────────────────────────────────────────────────────────────────────────────────────────┐" << std::endl;
	std::cout << "\t│ Delete a script previously loaded in MetaCall                                          │" << std::endl;
	std::cout << "\t│────────────────────────────────────────────────────────────────────────────────────────│" << std::endl;
	std::cout << "\t│ Usage:                                                                                 │" << std::endl;
	std::cout << "\t│ clear <runtime tag> <script0> <script1> ... <scriptN>                                  │" << std::endl;
	std::cout << "\t│    <runtime tag> : identifier to the type of script                                    │" << std::endl;
	std::cout << "\t│                  options :                                                             │" << std::endl;
	std::cout << "\t│                            mock - Mock (for testing purposes)                          │" << std::endl;
	std::cout << "\t│                            py   - Python                                               │" << std::endl;
	std::cout << "\t│                            node - NodeJS                                               │" << std::endl;
	std::cout << "\t│                            rb   - Ruby                                                 │" << std::endl;
	std::cout << "\t│                            cs   - C# NetCore                                           │" << std::endl;
	std::cout << "\t│                            cob  - Cobol                                                │" << std::endl;
	std::cout << "\t│                            ts   - TypeScript                                           │" << std::endl;
	std::cout << "\t│                            js   - V8 JavaScript Engine                                 │" << std::endl;
	std::cout << "\t│                            file - Files (for handling file systems)                    │" << std::endl;
	std::cout << "\t│    <script0> <script1> ... <scriptN> : id of the script (file name without extension)  │" << std::endl;
	std::cout << "\t│                                                                                        │" << std::endl;
	std::cout << "\t│ Example:                                                                               │" << std::endl;
	std::cout << "\t│ clear node concat                                                                      │" << std::endl;
	std::cout << "\t│                                                                                        │" << std::endl;
	std::cout << "\t│ Result:                                                                                │" << std::endl;
	std::cout << "\t│ Script (concat) removed correctly                                                      │" << std::endl;
	std::cout << "\t└────────────────────────────────────────────────────────────────────────────────────────┘" << std::endl
			  << std::endl;

	/* Exit command */
	std::cout << "\t┌────────────────────────────────────────────────────────────────────────────────────────┐" << std::endl;
	std::cout << "\t│ Exit from MetaCall CLI                                                                 │" << std::endl;
	std::cout << "\t│────────────────────────────────────────────────────────────────────────────────────────│" << std::endl;
	std::cout << "\t│ Usage:                                                                                 │" << std::endl;
	std::cout << "\t│ exit                                                                                   │" << std::endl;
	std::cout << "\t└────────────────────────────────────────────────────────────────────────────────────────┘" << std::endl
			  << std::endl;

	/* Help command */
	std::cout << "\t┌────────────────────────────────────────────────────────────────────────────────────────┐" << std::endl;
	std::cout << "\t│ Show help for MetaCall CLI                                                             │" << std::endl;
	std::cout << "\t│────────────────────────────────────────────────────────────────────────────────────────│" << std::endl;
	std::cout << "\t│ Usage:                                                                                 │" << std::endl;
	std::cout << "\t│ help                                                                                   │" << std::endl;
	std::cout << "\t└────────────────────────────────────────────────────────────────────────────────────────┘" << std::endl
			  << std::endl;

	return true;
}

bool command_cb_debug(application &app, tokenizer &t)
{
	std::cout << "[DEBUG]" << std::endl;

	app.command_debug(*t.begin(), t);

	return true;
}

bool command_cb_inspect(application &app, tokenizer &)
{
	size_t size = 0;

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	char *inspect_str = metacall_inspect(&size, allocator);

	if (inspect_str == NULL || size == 0)
	{
		std::cout << "(null)" << std::endl;
	}
	else
	{
		app.command_inspect(inspect_str, size, allocator);
	}

	if (inspect_str != NULL)
	{
		metacall_allocator_free(allocator, inspect_str);
	}

	metacall_allocator_destroy(allocator);

	return true;
}

bool command_cb_eval(application &app, tokenizer &t)
{
	tokenizer::iterator it = t.begin();

	parser p(it);

	std::string loader_tag;

	++it;

	if (p.is<std::string>())
	{
		loader_tag = *it;
	}

	std::string loaders[] = {
		"mock", "py", "node", "rb", "cs", "cob", "ts", "js", "file", "wasm"
	};

	// check if invalid loader tag
	if (std::find(std::begin(loaders), std::end(loaders), loader_tag) == std::end(loaders))
	{
		return false;
	}

	++it;

	if (!p.is<std::string>())
	{
		return false;
	}

	app.load_from_memory(loader_tag, *it);

	return true;
}

bool command_cb_call(application &app, tokenizer &t)
{
	const std::string func_delimiters(" \n\t\r\v\f(,)");

	tokenizer::iterator it = t.begin();

	/* Set custom function delimiters */
	t.delimit(func_delimiters);

	/* Skip command key */
	++it;

	/* Parse function call */
	if (it != t.end())
	{
		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		std::string func_name(*it);
		char *validate_name = const_cast<char *>(func_name.c_str());

		/* Check if function is loaded */
		if (metacall_function(validate_name) != NULL)
		{
			const std::string param_delimiters("()");

			/* Convert arguments into an array */
			std::string args = "[";

			t.delimit(param_delimiters);

			++it;

			if (it != t.end())
			{
				args += *it;
			}

			args += "]";

			/*
         void * result = app.metacallv_adaptor(func_name, args);
         */

			void *result = app.metacallfs_adaptor(func_name, args, allocator);

			if (result != NULL)
			{
				size_t size = 0;

				char *value_str = metacall_serialize(metacall_serial(), result, &size, allocator);

				std::cout << value_str << std::endl;

				metacall_allocator_free(allocator, value_str);

				metacall_value_destroy(result);
			}
			else
			{
				std::cout << "(null)" << std::endl;
			}

			metacall_allocator_destroy(allocator);
			return true;
		}

		else
		{
			std::cout << std::endl
					  << "Could not find function `" << func_name << "`" << std::endl;
			std::cout << "Make sure it is loaded" << std::endl
					  << std::endl;

			metacall_allocator_destroy(allocator);
			return false;
		}
	}

	return false;
}

bool command_cb_await(application &app, tokenizer &t)
{
	const std::string func_delimiters(" \n\t\r\v\f(,)");

	tokenizer::iterator it = t.begin();

	/* Set custom function delimiters */
	t.delimit(func_delimiters);

	/* Skip command key */
	++it;

	/* Parse function call */
	if (it != t.end())
	{
		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		std::string func_name(*it);

		const std::string param_delimiters("()");

		/* Convert arguments into an array */
		std::string args = "[";

		t.delimit(param_delimiters);

		++it;

		if (it != t.end())
		{
			args += *it;
		}

		args += "]";

		/*
		void * result = app.metacallfv_await_adaptor(func_name, args);
		*/

		void *result = app.metacallfs_await_adaptor(func_name, args, allocator);

		if (result != NULL)
		{
			size_t size = 0;

			char *value_str = metacall_serialize(metacall_serial(), result, &size, allocator);

			std::cout << value_str << std::endl;

			metacall_allocator_free(allocator, value_str);

			metacall_value_destroy(result);
		}
		else
		{
			std::cout << "(null)" << std::endl;
		}

		metacall_allocator_destroy(allocator);

		return true;
	}

	return false;
}

bool command_cb_load(application &app, tokenizer &t)
{
	tokenizer::iterator it = t.begin();

	parser p(it);

	std::string loader_tag;

	++it;

	if (p.is<std::string>())
	{
		loader_tag = *it;
	}

	std::string loaders[] = {
		"mock", "py", "node", "rb", "cs", "cob", "ts", "js", "file", "wasm"
	};

	// check if invalid loader tag
	if (std::find(std::begin(loaders), std::end(loaders), loader_tag) == std::end(loaders))
	{
		return false;
	}

	do
	{
		++it;

	} while (it != t.end() && p.is<std::string>() && app.load(loader_tag, *it));

	return true;
}

bool command_cb_clear(application &app, tokenizer &t)
{
	tokenizer::iterator it = t.begin();

	parser p(it);

	std::string loader_tag;

	++it;

	if (p.is<std::string>())
	{
		loader_tag = *it;
	}

	do
	{
		++it;

	} while (it != t.end() && p.is<std::string>() && app.clear(loader_tag, *it));

	return true;
}

bool command_cb_exit(application &app, tokenizer & /*t*/)
{
	std::cout << "Exiting ..." << std::endl;

	app.shutdown();

	return true;
}

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
		{ "tsx", "ts" }

		/* Note: By default js extension uses NodeJS loader instead of JavaScript V8 */
		/* Probably in the future we can differenciate between them, but it is not trivial */
	};

	const std::string tag = extension_to_tag[script.substr(script.find_last_of(".") + 1)];
	const std::string safeTag = tag != "" ? tag : "file"; /* Use File Loader if the tag is not found */

	app.load(safeTag, script);
	app.shutdown();
}

bool application::load(const std::string &tag, const std::string &script)
{
	const char *load_scripts[] = {
		script.c_str()
	};

	if (metacall_load_from_file(tag.c_str(), load_scripts, sizeof(load_scripts) / sizeof(load_scripts[0]), NULL) != 0)
	{
		std::cout << "Script (" << script << ") load error in loader (" << tag << ")" << std::endl;

		return false;
	}

	scripts.push_back(script);

	std::cout << "Script (" << script << ") loaded correctly" << std::endl;

	return true;
}

bool application::load_from_memory(const std::string &tag, const std::string &script)
{
	if (metacall_load_from_memory(tag.c_str(), script.c_str(), script.size() * sizeof(std::string::value_type), NULL) != 0)
	{
		std::cout << "Script '" << script << "' eval error in loader (" << tag << ")" << std::endl;

		return false;
	}

	std::cout << "Script '" << script << "' evaluated correctly" << std::endl;

	return true;
}

bool application::clear(const std::string &tag, const std::string &script)
{
	void *handle = metacall_handle(tag.c_str(), script.c_str());

	if (handle == NULL)
	{
		std::cout << "Script (" << script << ") not found in loader (" << tag << ")" << std::endl;

		return false;
	}

	if (metacall_clear(handle) != 0)
	{
		std::cout << "Script (" << script << ") clear error in loader (" << tag << ")" << std::endl;

		return false;
	}

	script_list::iterator it = std::find(scripts.begin(), scripts.end(), script);

	if (it != scripts.end())
	{
		scripts.erase(it);
	}

	std::cout << "Script (" << script << ") removed correctly" << std::endl;

	return true;
}

application::application(int argc, char *argv[]) :
	exit_condition(false)
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

	/* TODO: This has been updated, review it: */
	/* Parse program arguments if any (e.g metacall (0) a.py (1) b.js (2) c.rb (3)) */
	if (argc > 1)
	{
		parameter_iterator param_it(*this);

		/* TODO: This has been refactored in order to pass the arguments to the runtimes */
		/* Using argv + 2 by now, but this should be deleted in a near future or review the implementation */

		/* Parse program parameters */
		std::for_each(&argv[1], argv + /*argc*/ 2, param_it);
	}

	/* Define available commands */
	define("help", &command_cb_help);

	define("debug", &command_cb_debug);

	define("inspect", &command_cb_inspect);

	define("eval", &command_cb_eval);

	define("call", &command_cb_call);

	define("await", &command_cb_await);

	define("load", &command_cb_load);

	define("clear", &command_cb_clear);

	define("exit", &command_cb_exit);
}

application::~application()
{
	if (metacall_destroy() != 0)
	{
		std::cout << "Error while destroying MetaCall." << std::endl;
	}
}

void application::run()
{
	/* Show welcome message  */
	if (exit_condition != true)
	{
		std::cout << "Welcome to Tijuana, tequila, sexo & marijuana." << std::endl;
	}

	while (exit_condition != true)
	{
		std::string input;

/* Show prompt line */
#if defined(WIN32) || defined(_WIN32)
		/* TODO: Windows special characters not working properly */
		/* std::cout << L'\u03BB' << ' '; */
		std::cout << "> ";
#else
		std::cout << "\u03BB ";
#endif

		/* Get whole line */
		std::getline(std::cin, input);

		if (std::cin.eof() || std::cin.fail() || std::cin.bad())
		{
			shutdown();
			return;
		}

		/* Check for valid data */
		if (input.length() > 0)
		{
			/* Create tokenizer from input string */
			tokenizer t(input);

			/* Execute the command */
			execute(t);
		}
	}
}

void application::shutdown()
{
	exit_condition = true;
}

void application::command_debug(const std::string &key, const tokenizer &t)
{
	std::cout << "{" << std::endl
			  << "\tkey : " << key << "," << std::endl
			  << "\targuments :" << std::endl;

	for (tokenizer::iterator it = t.begin(); it != t.end(); ++it)
	{
		std::cout << "\t\t[" << it.position() << "] " << *it << std::endl;
	}

	std::cout << "}" << std::endl;
}

void value_array_for_each(void *v, const std::function<void(void *)> &lambda)
{
	void **v_array = static_cast<void **>(metacall_value_to_array(v));
	size_t count = metacall_value_count(v);

	std::for_each(v_array, v_array + count, lambda);
}

void value_map_for_each(void *v, const std::function<void(const char *, void *)> &lambda)
{
	void **v_map = static_cast<void **>(metacall_value_to_map(v));
	size_t count = metacall_value_count(v);

	std::for_each(v_map, v_map + count, [&lambda](void *element) {
		void **v_element = metacall_value_to_array(element);
		lambda(metacall_value_to_string(v_element[0]), v_element[1]);
	});
}

void application::command_inspect(const char *str, size_t size, void *allocator)
{
	void *v = metacall_deserialize(metacall_serial(), str, size, allocator);

	if (v == NULL)
	{
		std::cout << "Invalid deserialization" << std::endl;

		return;
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

						std::cout << metacall_value_to_string(v_arg_name_tupla[1]);

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
}

void application::execute(tokenizer &t)
{
	tokenizer::iterator it = t.begin();

	command_callback cb = commands[*it];

	if (cb == nullptr)
	{
		std::cout << "[WARNING]: Invalid command" << std::endl;

		command_debug(*it, t);

		std::cout << "See `help` for list of available commands" << std::endl;

		return;
	}

	if (cb(*this, t) == false)
	{
		std::cout << "[WARNING]: Invalid command execution" << std::endl;

		command_debug(*it, t);

		std::cout << "See `help` for correct usage" << std::endl;

		return;
	}
}

void application::define(const char *key, application::command_callback command_cb)
{
	std::string cmd(key);

	commands[cmd] = command_cb;
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

void *application::metacallv_adaptor(const std::string &name, const std::vector<void *> &args)
{
	void **args_ptr = new void *[args.size()];

	void *result = NULL;

	if (args_ptr != nullptr)
	{
		size_t iterator = 0;

		std::for_each(args.begin(), args.end(), [&iterator, args_ptr](void *v) {
			args_ptr[iterator] = v;

			++iterator;
		});

		result = metacallv(name.c_str(), args_ptr);

		delete[] args_ptr;
	}

	return result;
}

void *application::metacallfs_adaptor(const std::string &name, const std::string &args, void *allocator)
{
	void *func = metacall_function(name.c_str());

	return metacallfs(func, args.c_str(), args.length() + 1, allocator);
}

void *application::metacallfs_await_adaptor(const std::string &name, const std::string &args, void *allocator)
{
	void *func = metacall_function(name.c_str());
#if 0
	return metacallfs_await(func, args.c_str(), args.length() + 1, allocator, NULL, NULL, NULL);
#endif

	std::unique_lock<std::mutex> lock(this->await_mutex);

	struct await_data_type
	{
		void *v;
		application *app;
	};

	struct await_data_type data = { NULL, this };

	void *future = metacallfs_await(
		func, args.c_str(), args.length() + 1, allocator,
		[](void *result, void *ctx) -> void * {
			struct await_data_type *await_data = static_cast<struct await_data_type *>(ctx);
			std::unique_lock<std::mutex> lock(await_data->app->await_mutex);
			/* Value must be always copied, it gets deteled after the scope */
			await_data->v = metacall_value_copy(result);
			await_data->app->await_cond.notify_one();
			return NULL;
		},
		[](void *result, void *ctx) -> void * {
			struct await_data_type *await_data = static_cast<struct await_data_type *>(ctx);
			std::unique_lock<std::mutex> lock(await_data->app->await_mutex);
			/* Value must be always copied, it gets deteled after the scope */
			await_data->v = metacall_value_copy(result);
			await_data->app->await_cond.notify_one();
			return NULL;
		},
		static_cast<void *>(&data));

	this->await_cond.wait(lock);

	/* Unused */
	metacall_value_destroy(future);

	return data.v;
}
