#include <load_extensions/load_extensions.h>

#include <environment/environment_variable_path.h>
#include <log/log.h>
#include <metacall/metacall.h>

#include <assert.h>

#include <filesystem>
#include <regex>
#include <string>

#define METACALL_EXTENSIONS_PATH "METACALL_EXTENSIONS_PATH" /*ENV Variable for plugin path*/

namespace fs = std::filesystem;

std::string get_ext_path()
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

	char *lib_path = environment_variable_path_create(METACALL_EXTENSIONS_PATH, path, length + 1, NULL);
	if (!lib_path)
	{
		return "";
	}

	fs::path tmp(lib_path);
	environment_variable_path_destroy(lib_path);
	tmp /= "extensions";
	return tmp.string();
}

void load_extensions(void *loader, void *context)
{
	std::regex metacall_json{ R"(metacall(-.+)?\.json$)" };
	std::string ext_path = get_ext_path();
	if (ext_path.empty())
	{
		/*TODO: log*/
		assert(!"Failed to get metacall lib path");
	}

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };
	void *config_allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	auto i = fs::recursive_directory_iterator(ext_path);
	while (i != fs::recursive_directory_iterator())
	{
		if (i.depth() == 2)
			i.disable_recursion_pending();

		fs::directory_entry dir(*i);
		if (dir.is_regular_file())
		{
			if (std::regex_match(dir.path().filename().c_str(), metacall_json))
			{
				metacall_load_from_configuration(dir.path().c_str(), NULL, config_allocator);
				i.pop();
				continue;
			}
		}
		i++;
	}

	metacall_allocator_destroy(config_allocator);
}
