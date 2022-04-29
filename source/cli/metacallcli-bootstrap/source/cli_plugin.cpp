#include <metacallcli/cli_plugin.hpp>

#include <metacallcli/cli_options.hpp>

#include <dynlink/dynlink.h>

#include <environment/environment_variable_path.h>

#include <log/log.h>

#include <metacall/metacall.h>

#include <cstdlib>
#include <cstring>

#include <filesystem>
#include <string>

using namespace metacallcli;
namespace fs = std::filesystem;
#define CLI__PLUGIN_PATH	 "CLI_PLUGIN_PATH" /*ENV Variable for plugin path*/
#define CLI_PLUGIN_DIR		 "cli_plugins"
#define BASE_CLI_PLUGINS_DIR "base_plugins"

#define INIT_IFACE_SUFFIX "_init"
#define FINI_IFACE_SUFFIX "_fini"
#define MAIN_IFACE_SUFFIX "_main"

/* List of file extensions mapped into loader tags */
static std::unordered_map<std::string, std::string> extension_to_tag = {
	/*c loader*/
	{ "c", "c" },
	/* Cobol Loader */
	{ "cob", "cob" },
	{ "cbl", "cob" },
	{ "cpy", "cob" },
	/* C# Loader */
	{ "cs", "cs" },
	{ "dll", "cs" },
	{ "vb", "cs" },
	/* Mock Loader */
	{ "mock", "mock" },
	/* NodeJS Loader */
	{ "js", "node" },
	{ "node", "node" },
	/* Python Loader */
	{ "py", "py" },
	/* Ruby Loader */
	{ "rb", "rb" },
	/* Rust Loader */
	{ "rs", "rs" },
	/* TypeScript Loader */
	{ "ts", "ts" },
	{ "jsx", "ts" },
	{ "tsx", "ts" }

	/*Todo: add other loaders*/
	/* Note: By default js extension uses NodeJS loader instead of JavaScript V8 */
	/* Probably in the future we can differenciate between them, but it is not trivial */
};

std::string metacallcli::get_loader_tag(const std::string &ext)
{
	if (extension_to_tag.find(ext) != extension_to_tag.end())
		return extension_to_tag[ext];
	return ""; /*should check if return value is */
}

void metacallcli::get_plugin_path(std::string &pathref)
{
	/* Initialize the library path */
	const char name[] = "metacall"
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
						"d"
#endif
		;

	dynlink_library_path_str path;
	size_t length = 0;

	/* The order of precedence is:
  * 1) Environment variable
  * 2) Dynamic link library path of the host library
  */
	dynlink_library_path(name, path, &length); //TODO: check return value

	char *lib_path = environment_variable_path_create(CLI__PLUGIN_PATH, path, length + 1, NULL);
	if (lib_path)
	{
		fs::path tmp(lib_path);
		tmp /= CLI_PLUGIN_DIR;
		pathref = tmp.string();
		environment_variable_path_destroy(lib_path);
	}
}

void *metacallfv_adapter(const char *func_name, const void *handle, void *args[], size_t size)
{
	void *func = metacall_handle_function(const_cast<void *>(handle), func_name);
	if (func == NULL)
		return NULL;

	if (args == NULL || size == 0)
	{
		args = metacall_null_args;
	}

	void *value = NULL;
	value = metacallfv_s(func, args, size);
	return value;
}

PluginArch *PluginArch::plugin_arch_inst = NULL;

PluginArch::PluginArch()
{
	if (metacall_initialize() != 0)
	{
		log_write("metacall", LOG_LEVEL_CRITICAL, "metacall failed to initialize");
		/* Exit from application */
		std::_Exit(5);
	}

	metacallcli::get_plugin_path(plugin_path);
	base_plugin_path = fs::path(plugin_path) / BASE_CLI_PLUGINS_DIR;

	/*config parser needs to be loaded before every other plugin*/
	fs::path config_parser_path = fs::path(BASE_CLI_PLUGINS_DIR) / config_parser;
	LoadPlugin(config_parser_path.c_str());

	/*Todo make sure config parser is only loaded once*/
	//load all plugins in base_plugin_path
	load_base_plugins();
}

PluginArch *PluginArch::GetPluginArch()
{
	if (plugin_arch_inst == NULL)
	{
		plugin_arch_inst = new PluginArch();
	}

	return (plugin_arch_inst);
}

int PluginArch::Initialize()
{
	/*build list of all files in plugin folder*/
	for (const std::filesystem::directory_entry &dir_entry :
		std::filesystem::directory_iterator(plugin_path))
	{
		if (fs::status(dir_entry.path().c_str()).type() == fs::file_type::regular)
		{
			fs::path tmp(dir_entry);
			cmd_plugins.emplace(tmp.replace_extension().filename(),
				tmp.c_str());
		}
	}

	if (cmd_plugins.empty())
	{
		log_write("metacall", LOG_LEVEL_WARNING, "Unable to find any plugins in cli_plugins directory");
		return 1;
	}

	return 0;
}

PluginArch::~PluginArch()
{ /*is there cause for?*/
	for (auto &it : __plugins)
	{
		delete it.second;
	}
}

int PluginArch::LoadPlugin(const std::string &name)
{
	fs::path pl_path;
	fs::path cwd = fs::current_path();

	if (IsCmdPlugin(name)) /*if called with name of plugin*/
	{
		pl_path = this->cmd_plugins[name];
	}
	else if (fs::path(name).is_absolute()) /*else if called with an absolute path to plugin*/
	{
		pl_path = name;
	}
	else /*else called with relative path*/
	{
		fs::current_path(this->plugin_path);
		try
		{
			pl_path = fs::absolute(name); /*Todo check if it throws except*/
		}
		catch (std::filesystem::filesystem_error const &) /*recover from filesystem::filesystem_error*/
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Unable to find %s", name.c_str());
			/*reset current_path to CWD*/
			fs::current_path(cwd);
			return 1;
		}
	}

	if (fs::status(pl_path.c_str()).type() != fs::file_type::regular)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "%s is not a plugin", pl_path.c_str());
		return 1;
	}

	std::string ext = pl_path.extension();
	const std::string tag = extension_to_tag[ext.erase(0, 1)];
	void *handle = NULL;
	const char *load_scripts[] = {
		pl_path.c_str()
	};

	int ret = metacall_load_from_file(tag.c_str(), load_scripts, 1, &handle);
	if (ret == 0)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "Registering %s", pl_path.c_str());
		ret = register_plugin_impl(pl_path.replace_extension().filename(), handle);
	}
	else
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Unable to load: %s", pl_path.c_str());
	}

	/*reset current_path to CWD*/
	fs::current_path(cwd);
	return ret;
}

int PluginArch::ParseConfig(const std::string &name, void *jconf_value)
{
	if (!PluginIsLoaded(CONFIG_PARSER))
	{
		log_write("metacall", LOG_LEVEL_CRITICAL, "config parser not loaded.");
		_Exit(1);
	}

	if (!PluginIsLoaded(name))
		return 1;

	struct config_parser_func
	{
		const char *get_type;
		const char *get_apis;
		const char *get_options;
		const char *get_help_text;
		const char *supp_pos_args;
		const char *config_parser_check_config;
	};

	static struct config_parser_func conf_parser_func = {
		"config_parser_get_type", "config_parser_get_apis",
		"config_parser_get_options", "config_parser_get_help_txt",
		"config_parser_supports_pos_args", "config_parser_check_config"
	};

	void *errors = NULL;
	void *pos_args = NULL;
	void *help_text = NULL;

	plugin_desc const parser_desc = get_plugin_descriptor(CONFIG_PARSER);
	plugin_desc const pl_desc = get_plugin_descriptor(name);

	errors = metacallfv_adapter(conf_parser_func.config_parser_check_config, parser_desc->impl, &jconf_value, 1);
	if (errors)
	{
		if (std::string("JSON_ERROR") == metacall_value_to_string(errors))
		{
			/*Log*/
			return 1;
		}
	}

	if (pl_desc->type == kCommand)
	{
		pos_args = metacallfv_adapter(conf_parser_func.supp_pos_args, parser_desc->impl, &jconf_value, 1);
		help_text = metacallfv_adapter(conf_parser_func.get_help_text, parser_desc->impl, &jconf_value, 1);

		PluginOptions::AddOptions(std::string("--") + name,
			(help_text ? metacall_value_to_string(help_text) : ""),
			((pos_args) ? (std::string(metacall_value_to_string(pos_args)) == "true") ? kOption : kFlag : kFlag));

		//Todo loop through Plugins options and add extra plugins
	}
	else /*if pl->desc->type == kBase*/
	{
		//do nothing
	}
	return 0;
}

int PluginArch::load_base_plugins()
{
	fs::path bs_path(this->base_plugin_path);

	/*Loop through all files in base_plugins folder*/
	for (const std::filesystem::directory_entry &dir_entry :
		std::filesystem::directory_iterator{ bs_path })
	{
		if (fs::status(dir_entry.path().c_str()).type() == fs::file_type::regular)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "loading base plugin: %s.", dir_entry.path().c_str());
			LoadPlugin(dir_entry.path().c_str());
		}
		else
		{
			/*Todo Log*/
		}
	}
	return 1;
}

int PluginArch::register_plugin_impl(const std::string name, const void *const handle)
{
	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "passed NULL handle to PluginArch::register_plugin_impl.");
		return 1;
	}

	std::string func_init = name + INIT_IFACE_SUFFIX;
	std::string func_fini = name + FINI_IFACE_SUFFIX;
	std::string func_main = name + MAIN_IFACE_SUFFIX;

	if (!metacall_handle_function(const_cast<void *>(handle), func_init.c_str()))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Plugin %s does not implement %s_init ", name);
		return 1;
	}

	if (!metacall_handle_function(const_cast<void *>(handle), func_fini.c_str()))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Plugin %s does not implement %s_fini ", name);
		return 1;
	}

	plugin_desc descriptor = new struct plugin_descriptor;
	if (!descriptor)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "memory allocation failed"); //really necessary?
		return 1;
	}

	if (metacall_handle_function(const_cast<void *>(handle), func_main.c_str()))
	{
		descriptor->type = kCommand;
	}
	else
	{
		descriptor->type = kBase;
	}

	descriptor->impl = const_cast<void *>(handle);
	__plugins.try_emplace(name, descriptor);

	if (descriptor->type == kCommand)
	{
		void *value = metacallfv_adapter(func_init.c_str(), handle, NULL, 0);
		if (value == NULL)
			return 1;

		if (ParseConfig(name, const_cast<void *>(handle)) == 1)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Unable to parse conf JSON of %s", name);
			metacall_clear(const_cast<void *>(handle));
			return 1;
		}
	}

	return 0;
}

bool PluginArch::IsCmdPlugin(const std::string &name)
{
	if (this->cmd_plugins.find(name) != this->cmd_plugins.end()) /*name represents plugin in plugin folder*/
		return true;

	return false;
}

bool PluginArch::PluginIsLoaded(const std::string &name)
{
	if (__plugins.find(name) != __plugins.end())
	{
		return true;
	}

	return false;
}

plugin_desc PluginArch::get_plugin_descriptor(const std::string &name)
{
	if (PluginIsLoaded(name))
	{
		return __plugins[name];
	}
	else
	{
		return NULL;
	}
}

int PluginArch::destroy_plugin_descriptor(const std::string &name)
{ /*Todo clear handle*/
	if (__plugins.find(name) != __plugins.end())
	{
		plugin_desc hold = __plugins[name];
		__plugins.erase(name);
		std::free(hold->impl); //was malloc'ed
		delete hold;
		return 0;
	}

	return 1;
}

void **PluginArch::GenPluginValueArgs(int argc, char **argv)
{
	if (argc < 1 || argv == NULL)
	{
		return NULL;
	}
	void *argc_value = metacall_value_create_int(argc);
	if (argc_value == NULL)
	{
		return NULL;
	}

	void **tmp_svalue = new void *[argc];
	for (int i = 0; i < argc; i++)
	{
		if (tmp_svalue == NULL)
			return NULL;

		tmp_svalue[i] = metacall_value_create_string(argv[i], std::strlen(argv[i]));
	}

	void *argv_value = metacall_value_create_array(const_cast<const void **>(tmp_svalue), argc);
	void **args_vptr = new void *[2];
	if (args_vptr == NULL)
	{
		return NULL;
	}

	args_vptr[0] = argc_value;
	args_vptr[1] = argv_value;

	return args_vptr;
}

int PluginArch::RunPluginMain(const std::string &name, void **args_vptr)
{
	if (!PluginIsLoaded(name))
	{
		if (LoadPlugin(name) != 0)
		{
			//Todo Log
			return PLUGIN_FUNC_CALL_FAILED; /*so we don't clash with return from plugin call*/
		}
	}

	plugin_desc desc = get_plugin_descriptor(name);
	if (desc == NULL || desc->impl == NULL)
	{
		//Todo Log
		return PLUGIN_FUNC_CALL_FAILED;
	}
	std::string func_main = name + MAIN_IFACE_SUFFIX;
	int args_vptr_size = 2;
	void *ret_value = metacallfv_adapter(func_main.c_str(), desc->impl, args_vptr, args_vptr_size);

	if (ret_value != NULL)
		return metacall_value_to_int(ret_value);

	return 0;
}

void PluginArch::print__plugins_map()
{
	for (auto &i : __plugins)
	{
		std::cout << "key: " << i.first << ", type: " << i.second->type << "}" << '\n';
	}
}
