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

#include <filesystem>
#include <string>

#define METACALL_PLUGIN_PATH "METACALL_PLUGIN_PATH" /* Environment variable for plugin path */

namespace fs = std::filesystem;

static int plugin_extension_get_path(std::string &ext_path)
{
	/* Initialize the library path */
	const char name[] = "metacall"
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
						"d"
#endif
		;

	dynlink_library_path_str lib_path;
	size_t length = 0;

	/* The order of precedence is:
	* 1) Environment variable
	* 2) Dynamic link library path of the host library
	*/
	if (dynlink_library_path(name, lib_path, &length) != 0)
	{
		return 1;
	}

	char *env_path = environment_variable_path_create(METACALL_PLUGIN_PATH, lib_path, length + 1, NULL);

	if (env_path == NULL)
	{
		return 1;
	}

	fs::path plugin_path(env_path);
	environment_variable_path_destroy(env_path);
	plugin_path /= "plugins";
	ext_path = plugin_path.string();

	return 0;
}

int plugin_extension(void *loader, void *context)
{
	(void)loader;
	std::string ext_path;

	if (plugin_extension_get_path(ext_path) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Define the extension path with the environment variable " METACALL_PLUGIN_PATH);
		return 1;
	}

	static std::string m_begins = "metacall-";
	static std::string m_ends = ".json";

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };
	void *config_allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	auto i = fs::recursive_directory_iterator(ext_path);
	while (i != fs::recursive_directory_iterator())
	{
		if (i.depth() == 2)
		{
			i.disable_recursion_pending();
		}

		fs::directory_entry dir(*i);
		if (dir.is_regular_file())
		{
			std::string config = dir.path().filename().c_str();

			if (config == "metacall.json" ||
				(config.substr(0, m_begins.size()) == m_begins &&
					config.substr(config.size() - m_ends.size()) == m_ends))
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Loading extension: %s", dir.path().filename().c_str());

				if (metacall_load_from_configuration_ctx(dir.path().c_str(), context, config_allocator) != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Failed to load extension: %s", dir.path().c_str());
					return 1;
				}

				i.pop();
				continue;
			}
		}

		i++;
	}

	metacall_allocator_destroy(config_allocator);

	return 0;
}
