/*
 *	Loader Library by Parra Studios
 *	A plugin for loading cobol code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <cob_loader/cob_loader_impl.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>

#include <log/log.h>

#include <stdlib.h>

#include <libcob.h>

#include <map>
#include <string>

typedef struct loader_impl_cob_handle_type
{
	std::map<std::string, void *> funcs;

} * loader_impl_cob_handle;

static void *loader_impl_cob_ptr = NULL;

int function_cob_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

function_return function_cob_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	const char *name = function_name(func);

	(void)impl;

	if (size == 0)
	{
		return value_create_int(cobcall(name, 0, NULL));
	}
	else
	{
		void **cob_args = static_cast<void **>(malloc(sizeof(void *) * size));

		if (cob_args == NULL)
		{
			return NULL;
		}

		for (size_t arg_count = 0; arg_count < size; ++arg_count)
		{
			cob_args[arg_count] = value_data(args[arg_count]);
		}

		int result = cobcall(name, size, cob_args);

		free(cob_args);

		return value_create_int(result);
	}
}

function_return function_cob_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)size;
	(void)resolve_callback;
	(void)reject_callback;
	(void)context;

	return NULL;
}

void function_cob_interface_destroy(function func, function_impl impl)
{
	(void)func;
	(void)impl;
}

function_interface function_cob_singleton(void)
{
	static struct function_interface_type cob_interface = {
		&function_cob_interface_create,
		&function_cob_interface_invoke,
		&function_cob_interface_await,
		&function_cob_interface_destroy
	};

	return &cob_interface;
}

loader_impl_data cob_loader_impl_initialize(loader_impl impl, configuration config)
{
	(void)impl;
	(void)config;

	// TODO: The enviroment path trick wont be needed if
	// we implement cob_loader_impl_execution_path properly.
	// Remove this once we implement this properly (if needed)

	// Copy environment variables in order to resolve properly the scripts
	const char *scripts_path = getenv("LOADER_SCRIPT_PATH");

	if (scripts_path != NULL)
	{
		setenv("COB_LIBRARY_PATH", scripts_path, 1);
	}

	// Initialize cobol
	cobinit();

	// Register initialization
	loader_initialization_register(impl);

	return &loader_impl_cob_ptr;
}

int cob_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	(void)impl;
	(void)path;

	// TODO: Store the current path and append the new one each time
	// cob_set_library_path removes the previous one
	// cob_set_library_path(path);

	return 0;
}

loader_handle cob_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	loader_impl_cob_handle cob_handle = new loader_impl_cob_handle_type();

	(void)impl;

	if (cob_handle == nullptr)
	{
		return NULL;
	}

	for (size_t path_count = 0; path_count < size; ++path_count)
	{
		loader_naming_name module_name;

		if (loader_path_get_name(paths[path_count], module_name) > 1)
		{
			void *func = cob_resolve(module_name);

			if (func == NULL)
			{
				const char *error = cob_resolve_error();

				if (error == NULL)
				{
					error = "Unknown error";
				}

				log_write("metacall", LOG_LEVEL_ERROR, error);
			}
			else
			{
				cob_handle->funcs.insert(std::pair<std::string, void *>(std::string(module_name), func));
			}
		}
	}

	if (cob_handle->funcs.size() == 0)
	{
		delete cob_handle;
		return NULL;
	}

	return cob_handle;
}

loader_handle cob_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char *buffer, size_t size)
{
	(void)impl;
	(void)name;
	(void)buffer;
	(void)size;

	/* TODO */

	return NULL;
}

loader_handle cob_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	(void)impl;
	(void)path;

	/* TODO */

	return NULL;
}

int cob_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_cob_handle cob_handle = static_cast<loader_impl_cob_handle>(handle);

	(void)impl;

	if (cob_handle != nullptr)
	{
		// TODO: Is there any cob_resolve inverse function?

		delete cob_handle;
	}

	return 0;
}

int cob_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_cob_handle cob_handle = static_cast<loader_impl_cob_handle>(handle);

	scope sp = context_scope(ctx);

	(void)impl;

	for (const auto &func : cob_handle->funcs)
	{
		function f = function_create(func.first.c_str(), 0, func.second, &function_cob_singleton);

		scope_define(sp, function_name(f), value_create_function(f));
	}

	return 0;
}

int cob_loader_impl_destroy(loader_impl impl)
{
	(void)impl;

	// Destroy children loaders
	loader_unload_children(impl, 0);

	return cobtidy();
}
