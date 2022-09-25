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

#include <log/log.h>
#include <metacall/metacall.h>

#if defined __has_include
	#if __has_include(<filesystem>)
		#include <filesystem>
namespace fs = std::filesystem;
	#elif __has_include(<experimental/filesystem>)
		#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
	#else
		#error "Missing the <filesystem> header."
	#endif
#else
	#error "C++ standard too old for compiling this file."
#endif

#include <string>

void *plugin_load_from_path(size_t argc, void *args[], void *data)
{
	/* TODO: Improve return values with throwable in the future */
	(void)data;

	if (argc != 1 && argc != 2)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid number of arguments passed to plugin_load_from_path: %" PRIuS, argc);
		return metacall_value_create_int(1);
	}

	if (metacall_value_id(args[0]) != METACALL_STRING)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid first parameter passed to plugin_load_from_path, it requires string");
		return metacall_value_create_int(2);
	}

	if (argc == 2)
	{
		if (metacall_value_id(args[1]) != METACALL_PTR)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid second parameter passed to plugin_load_from_path, it requires pointer");
			return metacall_value_create_int(3);
		}
	}

	std::string ext_path(metacall_value_to_string(args[0]));
	void **handle_ptr = NULL;

	if (argc == 2)
	{
		handle_ptr = static_cast<void **>(metacall_value_to_ptr(args[1]));
	}

	static std::string m_begins = "metacall-";
	static std::string m_ends = ".json";

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };
	void *config_allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	auto i = fs::recursive_directory_iterator(ext_path);
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
				std::string dir_path = dir.path().string();

				log_write("metacall", LOG_LEVEL_DEBUG, "Loading plugin: %s", dir_path.c_str());

				if (metacall_load_from_configuration(dir_path.c_str(), handle_ptr, config_allocator) != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Failed to load plugin: %s", dir_path.c_str());
					return metacall_value_create_int(4);
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

	return metacall_value_create_int(0);
}

int plugin_extension(void *loader, void *handle, void *context)
{
	enum metacall_value_id arg_types[] = { METACALL_STRING, METACALL_PTR };
	(void)handle;
	return metacall_register_loaderv(loader, context, "plugin_load_from_path", plugin_load_from_path, METACALL_INT, sizeof(arg_types) / sizeof(arg_types[0]), arg_types);
}
