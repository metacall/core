/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

/* -- Headers -- */

#include "application.hpp"
#include "tokenizer.hpp"
#include "parser.hpp"

#include <metacall/metacall.h>

#include <algorithm>
#include <iostream>

/* -- Namespace Declarations -- */

using namespace metacallcli;

/* -- Private Methods -- */

bool command_cb_help(application & app, tokenizer & /*t*/)
{
	std::cout << "MetaCall Command Line Interface by Parra Studios" << std::endl;
	std::cout << "Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>" << std::endl;
	std::cout << std::endl << "A command line interface example as metacall wrapper" << std::endl;

	/* CLI usage */
	app.usage();

	/* Command list */
	std::cout << std::endl << "Command list :" << std::endl << std::endl;

	/* Call command */
	std::cout << "\tcall <expression>(<arg0>, <arg1>, ... , <argN>)" << std::endl;
	std::cout << "\t\texpression : alphanumeric string beginning by letter [method1, method2, hello]" << std::endl;
	std::cout << "\t\targuments  :" << std::endl;
	std::cout << "\t\t\tbool [true | false]," << std::endl;
	std::cout << "\t\t\tchar ['A', 'B', 'C']," << std::endl;
	std::cout << "\t\t\tint [234, 0, -16673]," << std::endl;
	std::cout << "\t\t\tlong [58L, 9857 ... 3942]," << std::endl;
	std::cout << "\t\t\tfloat [5.42f, 458E3f]," << std::endl;
	std::cout << "\t\t\tdouble [5.42, 458E12]," << std::endl;
	std::cout << "\t\t\tpointer [0x07d97e32]," << std::endl;
	std::cout << "\t\t\tstring [\"hello world\"]" << std::endl;

	/* Load command */
	std::cout << std::endl << "\tload <script0> <script1> ... <scriptN>" << std::endl;

	/* Exit command */
	std::cout << std::endl << "\texit" << std::endl;

	/* Help command */
	std::cout << std::endl << "\thelp" << std::endl;

	return true;
}

bool command_cb_debug(application & app, tokenizer & t)
{
	std::cout << "[DEBUG]" << std::endl;

	app.command_debug(*t.begin(), t);

	return true;
}

bool command_cb_call(application & app, tokenizer & t)
{
	const std::string func_delimiters(" \n\t\r\v\f(,)");

	tokenizer::iterator it = t.begin();

	parser_parameter p(it);

	std::vector<value> args;

	/* Set custom function delimiters */
	t.delimit(func_delimiters);

	/* Skip command key */
	++it;

	/* Parse function call */
	if (it != t.end())
	{
		std::string func_name(*it);

		const std::string param_delimiters("(,)");

		t.delimit(param_delimiters);

		++it;

		if (it != t.end())
		{
			const std::string param_escape(" \n\t\r\v\f");

			value v = NULL;

			do
			{
				it.escape(param_escape);

				v = app.argument_parse(p);

				if (v != NULL)
				{
					args.push_back(v);
				}
				else
				{
					std::cout << "\tinvalid argument : {" << *it << "}" << std::endl;
				}

				++it;

			} while (it != t.end());
		}

		value result = app.metacallv_adaptor(func_name, args);

		const size_t value_str_size = 0xFF;

		size_t length = 0;

		char value_str[value_str_size];

		value_stringify(result, value_str, value_str_size, &length);

		std::cout << "result : " << value_str << std::endl;

		value_destroy(result);

		std::for_each(args.begin(), args.end(), [](value v)
		{
			value_destroy(v);
		});

		return true;
	}

	return false;
}

bool command_cb_load(application & app, tokenizer & t)
{
	tokenizer::iterator it = t.begin();

	parser p(it);

	do
	{
		++it;

	} while (it != t.end() && p.is<std::string>() && app.load(*it));

	return true;
}

bool command_cb_exit(application & app, tokenizer & /*t*/)
{
	std::cout << "Exiting ..." << std::endl;

	app.shutdown();

	return true;
}

/* -- Methods -- */

application::parameter_iterator::parameter_iterator(application & app, application::script_list & scripts) :
	app(app), scripts(scripts)
{

}

application::parameter_iterator::~parameter_iterator()
{

}

bool application::load(const std::string & script)
{
	if (metacall_load_from_file(script.c_str()) == 0)
	{
		scripts.push_back(script);

		std::cout << "Script (" << script << ") loaded correctly" << std::endl;

		return true;
	}

	return false;
}

void application::parameter_iterator::operator()(const char * parameter)
{
	std::string script(parameter);

	if (app.load(script) == false)
	{
		std::cout << "Invalid parameter (" << parameter << ")" << std::endl;
	}
}

application::application(int argc, char * argv[]) : exit_condition(false)
{
	/* Print MetaCall information */
	metacall_print_info();

	/* Parse program arguments if any */
	if (argc <= 1)
	{
		std::cout << "Invalid number of arguments (" << argc << ")";

		if (argc == 1)
		{
			std::cout << " - { " << argv[0] << " }";
		}

		/* Print usage information */
		usage();

		/* Do not enter in main loop */
		shutdown();
	}
	else
	{
		parameter_iterator param_it(*this, scripts);

		/* Parse program parameters */
		std::for_each(&argv[1], argv + argc, param_it);
	}

	/* Define available commands */
	define("help", &command_cb_help);

	define("debug", &command_cb_debug);

	define("call", &command_cb_call);

	define("load", &command_cb_load);

	define("exit", &command_cb_exit);
}

application::~application()
{

}

void application::usage()
{
	std::cout << std::endl << "Usage: metacallcli <script0> <script1> ... <scriptN>" << std::endl;
}

void application::run()
{
	while (exit_condition != true)
	{
		std::string input;

		/* Show prompt line */
		std::cout << "> ";

		/* Get whole line */
		std::getline(std::cin, input);

		/* Check for valid data */
		if (input.length() > 0)
		{
			/* Create tokenizer from input string */
			tokenizer t(input);

			/* Execute the command */
			execute(t);
		}

	} while (exit_condition != true);
}

void application::shutdown()
{
	exit_condition = true;
}

void application::command_debug(const std::string & key, const tokenizer & t)
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

void application::execute(tokenizer & t)
{
	tokenizer::iterator it = t.begin();

	command_callback cb = commands[*it];

	if (cb == nullptr)
	{
		std::cout << "[WARNING]: Invalid command" << std::endl;

		command_debug(*it, t);

		return;
	}

	if (cb(*this, t) == false)
	{
		std::cout << "[WARNING]: Invalid command execution" << std::endl;

		command_debug(*it, t);

		return;
	}
}

void application::define(const char * key, application::command_callback command_cb)
{
	std::string cmd(key);

	commands[cmd] = command_cb;
}

value application::argument_parse(parser_parameter & p)
{
	if (p.is<bool>())
	{
		bool b = p.to<bool>();

		boolean bo = static_cast<boolean>(b);

		return value_create_bool(bo);
	}
	else if (p.is<char>())
	{
		char c = p.to<char>();

		return value_create_char(c);
	}
	else if (p.is<int>())
	{
		int i = p.to<int>();

		return value_create_int(i);
	}
	else if (p.is<long>())
	{
		long l = p.to<long>();

		return value_create_long(l);
	}
	else if (p.is<float>())
	{
		float f = p.to<float>();

		return value_create_float(f);
	}
	else if (p.is<double>())
	{
		double d = p.to<double>();

		return value_create_double(d);
	}
	else if (p.is<void *>())
	{
		void * ptr = p.to<void *>();

		return value_create_ptr(ptr);
	}
	else if (p.is<std::string>())
	{
		std::string str = p.to<std::string>();

		return value_create_string(str.c_str(), str.length());
	}

	return NULL;
}

value application::metacallv_adaptor(const std::string & name, const std::vector<value> & args)
{
	void ** args_ptr = new void * [args.size()];

	value result = NULL;

	if (args_ptr != nullptr)
	{
		size_t iterator = 0;

		std::for_each(args.begin(), args.end(), [&iterator, args_ptr](value v)
		{
			args_ptr[iterator] = v;

			++iterator;
		});

		result = metacallv(name.c_str(), args_ptr);

		delete args_ptr;
	}

	return result;
}
