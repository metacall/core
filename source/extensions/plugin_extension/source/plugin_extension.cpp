/*
 *	Extension Library by Parra Studios
 *	An extension for loading a folder of plugins based on metacall.json files.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

#include <plugin_extension/plugin_extension.h>

#include <environment/environment_variable_path.h>
#include <log/log.h>
#include <metacall/metacall.h>

#include <string.h>

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

typedef struct plugin_extension_data_type
{
	void *handle;
	std::string base_path;
	std::vector<std::string> exec_paths;
} * plugin_extension_data;

static plugin_extension_data ext_data = NULL;

static int plugin_extension_get_path(std::string &ext_path)
{
	/* Initialize the library path */
	const char name[] = "plugin_extension"
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
						"d"
#endif
		;

	dynlink_library_path_str lib_path;
	size_t length = 0;

	if (dynlink_library_path(name, lib_path, &length) != 0)
	{
		return 1;
	}

	fs::path plugin_path(lib_path);
	plugin_path /= "plugins";
	ext_path = plugin_path.string();

	return 0;
}

static int plugin_extension_load_path(std::string exec_path, void *handle)
{
	static std::string m_begins = "metacall-";
	static std::string m_ends = ".json";

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };
	void *config_allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	auto i = fs::recursive_directory_iterator(exec_path);
	while (i != fs::recursive_directory_iterator())
	{
		if (i.depth() == 1)
		{
			i.disable_recursion_pending();
		}

		fs::directory_entry dir(*i);
		if (dir.is_regular_file())
		{
			std::string config = dir.path().filename().string();

			if (config == "metacall.json" ||
				(config.substr(0, m_begins.size()) == m_begins &&
					config.substr(config.size() - m_ends.size()) == m_ends))
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Loading extension: %s", config.c_str());

				std::string dir_path = dir.path().string();

				if (metacall_load_from_configuration(dir_path.c_str(), &handle, config_allocator) != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Failed to load extension: %s", dir_path.c_str());
					return 1;
				}
				i++;
				if (i != fs::end(i) && i.depth() == 1)
				{
					i.pop();
				}
				continue;
			}
		}

		i++;
	}

	metacall_allocator_destroy(config_allocator);
	return 0;
}

void *plugin_extension_load(size_t argc, void *args[], void *data)
{
	if (ext_data == NULL)
	{
		return metacall_value_create_int(1);
	}
	(void)argc;
	(void)data;

	char *plugin = metacall_value_to_string(args[0]);

	if (portability_path_is_absolute(plugin, strlen(plugin)) == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Cannot load plugins outside these exec paths %s", ext_data->base_path.c_str());
		return metacall_value_create_int(1);
	}

	fs::path base_path(ext_data->base_path);
	fs::path plugin_path = base_path / plugin;
	if (plugin_extension_load_path(plugin_path.c_str(), ext_data->handle) != 0)
	{
		return metacall_value_create_int(1);
	}

	return metacall_value_create_int(0);
}

int plugin_extension(void *loader, void *handle, void *context)
{
	ext_data = new plugin_extension_data_type();
	ext_data->handle = handle;

	if (plugin_extension_get_path(ext_data->base_path) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Unable to get plugin_extension execution path");
		return 1;
	}
	enum metacall_value_id arg_types[] = { METACALL_STRING };
	int ret = metacall_register_loaderv(loader, context, "plugin_extension_load", plugin_extension_load, METACALL_INT, sizeof(arg_types) / sizeof(arg_types[0]), arg_types);
	if (ret != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to register plugin_extension_load");
	}

	if (plugin_extension_load_path(ext_data->base_path, ext_data->handle) != 0)
	{
		return 1;
	}

	return 0;
}
