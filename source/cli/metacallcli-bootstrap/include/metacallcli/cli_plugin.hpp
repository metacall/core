#ifndef CLI_PLUGIN_IMPL
#define CLI_PLUGIN_IMPL 1

#include <iostream>
#include <string>
#include <unordered_map>

#define CONFIG_PARSER "config_parser"

#define PLUGIN_FUNC_CALL_FAILED 11111

namespace metacallcli
{
/*get plugin directory*/
void get_plugin_path(std::string &);

//file ext to loader tag
std::string get_loader_tag(const std::string &ext);

enum plugin_type
{
	kBase,
	kCommand
};

typedef void *plugin_impl_handle;

typedef struct plugin_descriptor
{
	plugin_impl_handle impl;
	enum plugin_type type;
} * plugin_desc;

//singleton class
class PluginArch
{
public:
	static PluginArch *GetPluginArch();
	/*initialize*/
	int Initialize();
	int LoadPlugin(const std::string &);					//load plugins
	bool PluginIsLoaded(const std::string &);				//if a plugin is already loaded
	bool IsCmdPlugin(const std::string &);					//is name a key in __plugin_list
	plugin_desc get_plugin_descriptor(const std::string &); // get a plugin implementation by name
	void **GenPluginValueArgs(int argc, char **argv);		//generatate value args passed to plugins
	int RunPluginMain(const std::string &, void **);		//run <pluginname>_main with value args array
	void print__plugins_map();

private:
	PluginArch(); //constructor
	PluginArch(const PluginArch &) = delete;
	PluginArch &operator=(const PluginArch) = delete;

	~PluginArch(); // private destructors

	//private member functions
	int load_base_plugins();										// load all plugins in base plugin folder
	int register_plugin_impl(const std::string, const void *const); //register a new plugin implementation
	int destroy_plugin_descriptor(const std::string &);

	int ParseConfig(const std::string &, void *);

	//PluginArch instance
	static PluginArch *plugin_arch_inst;

	//variables
	std::string plugin_path;
	std::string base_plugin_path;

	//config parser
	std::string config_parser = "config_parser.js";

	/*plugin name without extension, full path*/
	std::unordered_map<std::string, std::string> cmd_plugins;

	/*plugin name, plugin descriptor*/
	std::unordered_map<std::string, plugin_desc> __plugins;
}; //class PluginArch

} //namespace metacallcli
#endif
