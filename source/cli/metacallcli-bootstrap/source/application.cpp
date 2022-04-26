/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

/* -- Headers -- */

#include <metacallcli/application.hpp>

#include <algorithm>
#include <functional>
#include <iostream>

/* -- Namespace Declarations -- */

using namespace metacallcli;

application::application(int argc, char *argv[]) :
	exit_condition(false)
{
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
