/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

/* -- Headers -- */
#include <metacallcli/cli_options.hpp>
#include <metacallcli/cli_plugin.hpp>

#include <metacall/metacall.h>

#include <cstdlib>

#include <filesystem>
#include <iostream>

#include <string>

/* -- Namespace Declarations -- */

using namespace metacallcli;
namespace fs = std::filesystem;

/*privar*/
static int shutdown(int exitcode = 0, char *reason = NULL);

int cli_main(int argc, char **argv)
{
	/*declare error vector*/
	std::vector<std::string> errors;

	PluginArch *pa_inst = PluginArch::GetPluginArch();
	//pa_inst->load_plugin("test_cmd.js");
	pa_inst->print__plugins_map();

	PluginOptions plOptions;
	MetacallOptions cliOptions;
	EnvironmentOptions envOptions;

	Parser parser(argc, argv, &errors, &plOptions, &envOptions, &cliOptions);

	if (!errors.empty())
	{
		//Todo log errors to stderr
		return 1;
	}

	if (cliOptions.print_help)
	{
		std::cout << metacall_usage << '\n';
		shutdown();
	}

	if (cliOptions.print_version)
	{
		std::cout << "metacall (metacallcli-bootstrap) " << metacall_version_str() << '\n';
		shutdown();
	}

	if (parser.parsed_plopts)
	{
		if (plOptions.plugin_accepts_args)
		{
			void **args_v = pa_inst->GenPluginValueArgs(plOptions.get_plugin_argc(), plOptions.get_plugin_argv());
			if (args_v == NULL)
			{
				/**log
				 * proceed to call plugins main function*/
			}

			int ret = pa_inst->RunPluginMain(plOptions.get_plugin_name(), args_v);
			if (ret != 0)
			{
				/**
				 * Log more appropriate error messages to std::out*/
				return ret;
			}
		}
	}
	else if (parser.inc_fileargs)
	{
		std::vector<std::string> args;
		parser.get_file_args(&args);
		if (args.empty())
		{
			/**should be an error?*/
			return 0;
		}

		for (auto &i : args)
		{
			const char *load_scripts[] = {
				i.c_str()
			};

			fs::path filepath(i);
			std::string ext = filepath.extension();
			ext.erase(0, 1); /*remove leading '.' from extension*/
			const std::string tag = get_loader_tag(ext);

			if (metacall_load_from_file(tag.c_str(), load_scripts, sizeof(load_scripts) / sizeof(load_scripts[0]), NULL) != 0)
			{
				std::cout << "Script (" << i << ") load error in loader (" << tag << ")" << std::endl;

				return 1;
			}
		}
	}
	return 0;
}

static int shutdown(int exitcode, char *reason)
{
	/**
	 * should suffice for now*/
	if (reason)
	{
		std::cout << reason << '\n';
	}

	std::exit(exitcode);
}