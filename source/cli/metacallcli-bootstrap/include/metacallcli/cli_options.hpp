#ifndef CLI_OPTIONS
#define CLI_OPTIONS 1

#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

//
#define FLAG   (1 << 0) /*plugin name is a flag on the command line*/
#define OPTION (1 << 2) /*plugin name is an option on the command line*/
//#define NO_PARSE_OPT (1<<4) /*don't parse options, passthrough to plugins*/

extern const char *metacall_usage;

namespace metacallcli
{
//class PluginArch;

enum ArgType
{
	kNull,
	kBoolean,
	kInteger,
	kUInteger,
	kString,
	kStringList
};

enum OptionType
{
	kOption,
	kFlag,
	kPositional
};

typedef struct plugin_options
{
	std::string name;
	std::string help_text;
	enum OptionType opt_type;
} * plugin_options_st;

class Options
{
public:
	Options() {}
	Options(const Options &) = delete;
	Options &operator=(const Options &) = delete;
};

class PluginOptions : public Options
{
public:
	bool print_help = false;
	bool cmd_plugin = false;
	bool plugin_accepts_args = false;

	int CheckOptions(int argc, char **argv);
	std::string get_plugin_name();
	char **get_plugin_argv();
	int get_plugin_argc();

	static int AddOptions(std::string name, std::string help_text, enum OptionType opt_type);

private:
	/*private methods*/
	plugin_options_st GetOptionStruct(std::string);
	bool AcceptsArgs(std::string);

	/**/
	static std::unordered_map<std::string, plugin_options_st> __plugin_options;

	char **plugin_argv = NULL;
	int plugin_argc = 0;
	std::string plugin_name;
};

class MetacallOptions : public Options
{
public:
	bool print_help = false;
	bool print_version = false;
	int CheckOptions(char **argv, size_t size, std::vector<std::string> *errors);
};

class EnvironmentOptions : public Options
{
public:
	int GetEnvOption(std::string &);
	int CheckOptions(std::vector<std::string> *errors);
};

/*Parser*/

class Parser
{
public:
	/*constructors*/
	Parser(int argc, char **argv, std::vector<std::string> *errors,
		PluginOptions *PluginOptionsPtr, EnvironmentOptions *EnvOptionsPtr,
		MetacallOptions *MetacallOptionsPtr);
	Parser(Parser &) = delete;
	Parser &operator=(Parser &) = delete;
	~Parser() = default; //implement

	/*public member functions*/
	int Parse();
	int get_file_args(std::vector<std::string> *fileargs);

	bool parsed_mopts = false;
	bool parsed_envopts = false;
	bool parsed_plopts = false;
	bool inc_fileargs = false;

private:
	/*private member functions*/

	/*private variables*/
	char **argv;
	size_t argc;
	int margs_st_idx = 0; //metacall options start index in argv

	PluginOptions *PluginOptionsPtr;
	EnvironmentOptions *EnvOptionsPtr;
	MetacallOptions *MetacallOptionsPtr;
	std::vector<std::string> *errors;

}; //class Parser

} // namespace metacallcli
#endif
