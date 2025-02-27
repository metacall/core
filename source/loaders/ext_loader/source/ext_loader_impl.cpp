/*
 *	Loader Library by Parra Studios
 *	A plugin for loading extension code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <ext_loader/ext_loader_impl.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <portability/portability_path.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>

#include <dynlink/dynlink.h>

#include <log/log.h>

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

#include <map>
#include <set>
#include <string>
#include <vector>

typedef struct loader_impl_ext_handle_lib_type
{
	std::string name;
	dynlink handle;
	dynlink_symbol_addr addr;

} * loader_impl_ext_handle_lib;

typedef struct loader_impl_ext_type
{
	std::set<fs::path> paths;
	std::map<std::string, loader_impl_ext_handle_lib_type> destroy_list;

} * loader_impl_ext;

typedef struct loader_impl_ext_handle_type
{
	std::vector<loader_impl_ext_handle_lib_type> extensions;

} * loader_impl_ext_handle;

union loader_impl_function_cast
{
	dynlink_symbol_addr ptr;
	int (*fn)(void *, void *);
};

static dynlink ext_loader_impl_load_from_file_dynlink(const char *path, const char *library_name);
static dynlink ext_loader_impl_load_from_file_dynlink(loader_impl_ext ext_impl, const loader_path path);
static int ext_loader_impl_load_from_file_handle(loader_impl_ext ext_impl, loader_impl_ext_handle ext_handle, const loader_path path);
static void ext_loader_impl_destroy_handle(loader_impl_ext_handle ext_handle);

int ext_loader_impl_initialize_types(loader_impl impl)
{
	for (type_id index = 0; index < TYPE_SIZE; ++index)
	{
		type t = type_create(index, type_id_name(index), NULL, NULL);

		if (t != NULL)
		{
			if (loader_impl_type_define(impl, type_name(t), t) != 0)
			{
				type_destroy(t);
				return 1;
			}
		}
	}

	return 0;
}

loader_impl_data ext_loader_impl_initialize(loader_impl impl, configuration config)
{
	loader_impl_ext ext_impl = new loader_impl_ext_type();

	(void)impl;
	(void)config;

	if (ext_impl == nullptr)
	{
		return NULL;
	}

	if (ext_loader_impl_initialize_types(impl) != 0)
	{
		delete ext_impl;

		return NULL;
	}

	/* Register initialization */
	loader_initialization_register(impl);

	return static_cast<loader_impl_data>(ext_impl);
}

int ext_loader_impl_execution_path(loader_impl impl, const loader_path path)
{
	loader_impl_ext ext_impl = static_cast<loader_impl_ext>(loader_impl_get(impl));

	ext_impl->paths.insert(fs::path(path));

	return 0;
}

dynlink ext_loader_impl_load_from_file_dynlink(const char *path, const char *library_name)
{
	/* This function will try to check if the library exists before loading it,
	so we avoid error messages from dynlink when guessing the file path for relative load from file */
	dynlink_name_impl platform_name;

	dynlink_platform_name(library_name, platform_name);

	fs::path lib_path(path);

	lib_path /= platform_name;

	if (fs::exists(lib_path) == false)
	{
		return NULL;
	}

	return dynlink_load(path, library_name, DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);
}

dynlink ext_loader_impl_load_from_file_dynlink(loader_impl_ext ext_impl, const loader_path path)
{
	std::string lib_path_str(path);

#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	lib_path_str.append("d");
#endif

	fs::path lib_path(lib_path_str);

	if (lib_path.is_absolute())
	{
		fs::path lib_dir = lib_path.parent_path();
		std::string lib_name = fs::path(lib_path).filename().string();

		return ext_loader_impl_load_from_file_dynlink(lib_dir.string().c_str(), lib_name.c_str());
	}
	else
	{
		for (auto exec_path : ext_impl->paths)
		{
			fs::path absolute_path(exec_path);

			absolute_path /= lib_path.parent_path();

			std::string lib_name = lib_path.filename().string();

			dynlink lib = ext_loader_impl_load_from_file_dynlink(absolute_path.string().c_str(), lib_name.c_str());

			if (lib != NULL)
			{
				return lib;
			}
		}
	}

	return NULL;
}

int ext_loader_impl_load_from_file_handle(loader_impl_ext ext_impl, loader_impl_ext_handle ext_handle, const loader_path path)
{
	auto iterator = ext_impl->destroy_list.find(path);

	if (iterator != ext_impl->destroy_list.end())
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "Unloading handle: %s <%p>", iterator->second.name.c_str(), iterator->second.handle);
		dynlink_unload(iterator->second.handle);
		ext_impl->destroy_list.erase(path);
	}

	dynlink lib = ext_loader_impl_load_from_file_dynlink(ext_impl, path);

	if (lib == NULL)
	{
		ext_loader_impl_destroy_handle(ext_handle);

		log_write("metacall", LOG_LEVEL_ERROR, "Failed to load extension: %s", path);

		return 1;
	}

	dynlink_symbol_addr symbol_address = NULL;
	std::string symbol_name = fs::path(path).filename().string();

	if (dynlink_symbol(lib, symbol_name.c_str(), &symbol_address) != 0)
	{
		ext_loader_impl_destroy_handle(ext_handle);

		log_write("metacall", LOG_LEVEL_ERROR, "Failed to load symbol from extension: %s", path);

		return 1;
	}

	loader_impl_ext_handle_lib_type ext_handle_lib = { path, lib, symbol_address };

	ext_handle->extensions.push_back(ext_handle_lib);

	return 0;
}

loader_handle ext_loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size)
{
	loader_impl_ext ext_impl = static_cast<loader_impl_ext>(loader_impl_get(impl));

	if (ext_impl == NULL)
	{
		return NULL;
	}

	loader_impl_ext_handle ext_handle = new loader_impl_ext_handle_type();

	if (ext_handle == nullptr)
	{
		return NULL;
	}

	for (size_t iterator = 0; iterator < size; ++iterator)
	{
		if (ext_loader_impl_load_from_file_handle(ext_impl, ext_handle, paths[iterator]) != 0)
		{
			return NULL;
		}
	}

	return ext_handle;
}

loader_handle ext_loader_impl_load_from_memory(loader_impl impl, const loader_name name, const char *buffer, size_t size)
{
	(void)impl;
	(void)name;
	(void)buffer;
	(void)size;

	/* TODO: Here we should load the symbols from the process itself */

	return NULL;
}

loader_handle ext_loader_impl_load_from_package(loader_impl impl, const loader_path path)
{
	loader_impl_ext ext_impl = static_cast<loader_impl_ext>(loader_impl_get(impl));

	if (ext_impl == NULL)
	{
		return NULL;
	}

	loader_impl_ext_handle ext_handle = new loader_impl_ext_handle_type();

	if (ext_handle == nullptr)
	{
		return NULL;
	}

	if (ext_loader_impl_load_from_file_handle(ext_impl, ext_handle, path) != 0)
	{
		return NULL;
	}

	return ext_handle;
}

void ext_loader_impl_destroy_handle(loader_impl_ext_handle ext_handle)
{
	for (auto ext : ext_handle->extensions)
	{
		if (ext.handle != NULL)
		{
			dynlink_unload(ext.handle);
		}
	}

	delete ext_handle;
}

int ext_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_ext ext_impl = static_cast<loader_impl_ext>(loader_impl_get(impl));

	if (ext_impl == NULL)
	{
		return 1;
	}

	loader_impl_ext_handle ext_handle = static_cast<loader_impl_ext_handle>(handle);

	if (ext_handle != NULL)
	{
		for (size_t i = 0; i < ext_handle->extensions.size(); i++)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Storing handle: %s <%p> in destroy list", ext_handle->extensions[i].name.c_str(), ext_handle->extensions[i].handle);
			ext_impl->destroy_list[ext_handle->extensions[i].name] = ext_handle->extensions[i];
			ext_handle->extensions.erase(ext_handle->extensions.begin() + i);
		}

		delete ext_handle;

		return 0;
	}

	return 1;
}

int ext_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_ext_handle ext_handle = static_cast<loader_impl_ext_handle>(handle);

	(void)ctx; /* We have opted by pass the handle instead of the context of the handle */

	for (auto ext : ext_handle->extensions)
	{
		loader_impl_function_cast function_cast;

		function_cast.ptr = ext.addr;

		if (function_cast.fn(impl, loader_impl_handle_container_of(impl, handle)) != 0)
		{
			return 1;
		}
	}

	return 0;
}

int ext_loader_impl_destroy(loader_impl impl)
{
	loader_impl_ext ext_impl = static_cast<loader_impl_ext>(loader_impl_get(impl));

	if (ext_impl != NULL)
	{
		/* Destroy children loaders */
		loader_unload_children(impl);

		/* Destroy all handles */
		if (!ext_impl->destroy_list.empty())
		{
			for (auto iterator : ext_impl->destroy_list)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Unloading handle: %s <%p>", iterator.second.name.c_str(), iterator.second.handle);
				dynlink_unload(iterator.second.handle);
			}
		}

		delete ext_impl;

		return 0;
	}

	return 1;
}
