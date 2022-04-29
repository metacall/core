#include <metacallcli/cli_options.hpp>

#include <metacallcli/cli_plugin.hpp>

#include <metacall/metacall.h>

#include <assert.h>
#include <cstring>

#include <iostream>

using namespace metacallcli;

std::unordered_map<std::string, plugin_options_st> PluginOptions::__plugin_options;

const char *metacall_usage =
	"usage: metacallcli  \n"
	"                   [--help] [--version]\n"
	"                   <file> [<file> ...] [--<metacall-option> ...] [--<metacall-option> <value> ...]\n"
	"                   <command> [<args>] [--<metacall-option> ...] [--<metacall-option> <value> ...]\n"
	"\nMetaCall: The ultimate polyglot programming experience.\n";

int PluginOptions::AddOptions(std::string name, std::string help_text, enum OptionType opt_type)
{
	plugin_options_st tmp = new struct plugin_options;
	if (tmp == NULL)
		return 1;
	tmp->name = name;
	tmp->help_text = help_text;
	tmp->opt_type = opt_type;
	__plugin_options.emplace(name, tmp);
	return 0; //remove
}

int PluginOptions::CheckOptions(int argc, char **argv)
{
	std::string pname = std::string(argv[0]).erase(0, 2);
	PluginArch *pa_inst = PluginArch::GetPluginArch();
	if (!pa_inst->IsCmdPlugin(pname))
	{
		return 1;
	}

	cmd_plugin = true;
	if (__plugin_options.find(argv[0]) != __plugin_options.end())
	{
		/*Plugin exists hence load if not already loaded*/
		pa_inst->LoadPlugin(pname);
	}

	if (argc > 1)
	{
		if (std::strcmp("--help", argv[1]) == 0 || std::strcmp("help", argv[1]) == 0)
		{
			print_help = true;
			return 0;
		}

		if (AcceptsArgs(std::string(argv[0])))
		{
			plugin_accepts_args = true;
		}
	}

	/**
	 * the plan is to do some typechecking for plugin argv
	 * but would be too complex to do right now.
	 * So I indeed to hold of on this until we have a functioning cli
	 * */

	plugin_argv = argv;
	plugin_argc = argc;
	plugin_name = pname;
	return 0;
}

bool PluginOptions::AcceptsArgs(std::string option)
{
	plugin_options_st tmp = __plugin_options[option];
	if (tmp->opt_type == kOption)
		return true;

	return false;
}

plugin_options_st PluginOptions::GetOptionStruct(std::string option)
{
	if (option[0] != '-' && option[1] != '-')
	{
		option = std::string("--") + option;
	}

	if (__plugin_options.find(option) != __plugin_options.end())
	{
		return __plugin_options[option];
	}
	return NULL;
}

int PluginOptions::get_plugin_argc()
{
	return plugin_argc;
}

char **PluginOptions::get_plugin_argv()
{
	return plugin_argv;
}

std::string PluginOptions::get_plugin_name()
{
	return plugin_name;
}

int MetacallOptions::CheckOptions(char **argv, size_t size, std::vector<std::string> *errors)
{
	if (std::strcmp("--help", *argv) == 0 || std::strcmp("help", *argv) == 0)
	{
		this->print_help = true;
	}
	else if (std::strcmp("--version", *argv) == 0 || std::strcmp("version", *argv) == 0)
	{
		this->print_version = true;

		/**
	 * If argv[1] != --help, --version
	 * Iterate through argv and __metacall_options
	 * If argv[1] IsCmdPlugin() cmd args end at index [i] where *argv 
	 * matches option in __metacall_options
	 * Else if list of files to load end at index where *argv first match option in
	 * __metcall_options
	 * return index where first metacall option was found
	 * if none return -1
	 * if argv contains only metacall options, proceed, return 0;
	 * */
	}
	else
	{
		return -1;
	}

	return 0;
}

int EnvironmentOptions::CheckOptions(std::vector<std::string> *errors)
{
	//Todo
	return 0; //so gcc stop's complaining
}

bool IsOption(const char *arg)
{
	if (*arg != '\0' && *arg == '-')
	{
		arg++;
		if (*arg != '\0' && *arg == '-')
			return true;
	}

	return false;
}

Parser::Parser(int argc, char **argv, std::vector<std::string> *errors,
	PluginOptions *PluginOptionsPtr, EnvironmentOptions *EnvOptionsPtr,
	MetacallOptions *MetacallOptionsPtr) :
	argc(--argc)
{
	if (this->argc == 0)
	{
		assert(!"Called Parser constructor with 0 arguments");
	}

	if (!argv || !errors || !PluginOptionsPtr || !EnvOptionsPtr || !MetacallOptionsPtr)
		assert(!"Passed a NULL value to Parser Constructor");

	this->argv = ++argv;
	this->errors = errors;
	this->PluginOptionsPtr = PluginOptionsPtr;
	this->EnvOptionsPtr = EnvOptionsPtr;
	this->MetacallOptionsPtr = MetacallOptionsPtr;

	Parse();
}

int Parser::Parse()
{
	//EnvOptionsPtr->CheckOptions(errors);
	this->margs_st_idx = MetacallOptionsPtr->CheckOptions(this->argv, this->argc, this->errors);
	if (this->margs_st_idx >= 0)
	{
		parsed_mopts = true;
	}

	if (this->margs_st_idx == -1)
	{ //no metacall options
		std::cout << "margs_st_idx = " << this->margs_st_idx << '\n';
		if (PluginOptionsPtr->CheckOptions(this->argc, this->argv) == 0)
		{
			parsed_plopts = true;
		}
	}
	else if (this->margs_st_idx > 0)
	{
		if (PluginOptionsPtr->CheckOptions(this->margs_st_idx, this->argv) == 0)
		{ //tests
			parsed_plopts = true;
		}
	}

	if (!parsed_mopts && margs_st_idx != 0)
	{
		if (!IsOption(argv[0]))
		{ //need a better design
			inc_fileargs = true;
		}
	}

	if (this->margs_st_idx == -1 && !parsed_plopts && !inc_fileargs)
	{
		//Todo
		//errors->push_back("")
	}

	return 0;
}

int Parser::get_file_args(std::vector<std::string> *fileargs)
{
	if (this->inc_fileargs)
	{
		size_t size = margs_st_idx == -1 ? this->argc : margs_st_idx;
		for (size_t i = 0; i < size; i++)
		{
			fileargs->push_back(argv[i]);
		}
		return 0;
	}

	return 1;
}