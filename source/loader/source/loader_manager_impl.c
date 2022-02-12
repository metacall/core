/*
 *	Loader Library by Parra Studios
 *	A library for loading executable code at run-time into a process.
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

/* -- Headers -- */

#include <loader/loader_manager_impl.h>

#include <loader/loader_host.h>

#include <environment/environment_variable_path.h>

#include <portability/portability_executable_path.h>
#include <portability/portability_path.h>

#include <log/log.h>

#include <string.h>

/* -- Definitions -- */

#define LOADER_SCRIPT_PATH		   "LOADER_SCRIPT_PATH"
#define LOADER_SCRIPT_DEFAULT_PATH "."

/* -- Private Methods -- */

static vector loader_manager_impl_script_paths_initialize(void);

static void loader_manager_impl_iface_destroy(plugin_manager manager, void *impl);

static void loader_manager_impl_script_paths_destroy(vector script_paths);

/* -- Methods -- */

vector loader_manager_impl_script_paths_initialize(void)
{
	portability_executable_path_str exe_path_str = { 0 };
	portability_executable_path_length exe_path_str_length = 0;
	char *script_path = NULL;
	size_t script_path_size = 0;
	vector script_paths = vector_create_type(char *);

	if (script_paths == NULL)
	{
		return NULL;
	}

	if (portability_executable_path(exe_path_str, &exe_path_str_length) == 0)
	{
		size_t exe_directory_size = portability_path_get_directory_inplace(exe_path_str, exe_path_str_length + 1);

		script_path = environment_variable_path_create(LOADER_SCRIPT_PATH, exe_path_str, exe_directory_size, &script_path_size);
	}
	else
	{
		script_path = environment_variable_path_create(LOADER_SCRIPT_PATH, LOADER_SCRIPT_DEFAULT_PATH, sizeof(LOADER_SCRIPT_DEFAULT_PATH), &script_path_size);
	}

	log_write("metacall", LOG_LEVEL_DEBUG, "Loader script path: %s", script_path);

	/* Split multiple paths */
	size_t iterator, last = 0;

	for (iterator = 0; iterator < script_path_size; ++iterator)
	{
		if (script_path[iterator] == PORTABILITY_PATH_DELIMITER || script_path[iterator] == '\0')
		{
			script_path[iterator] = '\0';
			vector_push_back_empty(script_paths);
			char **script_path_ptr = vector_back(script_paths);
			size_t size = iterator - last;

			*script_path_ptr = malloc(sizeof(char) * size);

			if (*script_path_ptr == NULL)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Loader script path failed to allocate: %s", &script_path[last]);
			}
			else
			{
				memcpy(*script_path_ptr, &script_path[last], size);
			}

			last = iterator + 1;
		}
	}

	environment_variable_path_destroy(script_path);

	return script_paths;
}

void loader_manager_impl_script_paths_destroy(vector script_paths)
{
	size_t iterator, size = vector_size(script_paths);

	for (iterator = 0; iterator < size; ++iterator)
	{
		char *path = vector_at_type(script_paths, iterator, char *);

		free(path);
	}

	vector_destroy(script_paths);
}

loader_manager_impl loader_manager_impl_initialize(void)
{
	loader_manager_impl manager_impl = malloc(sizeof(struct loader_manager_impl_type));

	if (manager_impl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader failed to allocate its implementation");
		goto alloc_error;
	}

	manager_impl->initialization_order = vector_create(sizeof(struct loader_initialization_order_type));

	if (manager_impl->initialization_order == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader failed to allocate the initialization order vector");
		goto initialization_order_error;
	}

	manager_impl->script_paths = loader_manager_impl_script_paths_initialize();

	if (manager_impl->script_paths == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader failed to initialize the script paths");
		goto script_paths_error;
	}

	manager_impl->init_thread_id = thread_id_get_current();

	manager_impl->host = loader_host_initialize();

	if (manager_impl->host == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Loader failed to initialize the host loader");
		goto host_error;
	}

	return manager_impl;

host_error:
	loader_manager_impl_script_paths_destroy(manager_impl->script_paths);
script_paths_error:
	vector_destroy(manager_impl->initialization_order);
initialization_order_error:
	free(manager_impl);
alloc_error:
	return NULL;
}

plugin_manager_interface loader_manager_impl_iface(void)
{
	static struct plugin_manager_interface_type iface = {
		NULL, /* TODO? */
		&loader_manager_impl_iface_destroy
	};

	return &iface;
}

void loader_manager_impl_iface_destroy(plugin_manager manager, void *impl)
{
	loader_manager_impl manager_impl = (loader_manager_impl)impl;

	(void)manager;

	loader_manager_impl_destroy(manager_impl);
}

void loader_manager_impl_destroy(loader_manager_impl manager_impl)
{
	if (manager_impl != NULL)
	{
		if (manager_impl->initialization_order != NULL)
		{
			vector_destroy(manager_impl->initialization_order);
		}

		manager_impl->init_thread_id = THREAD_ID_INVALID;

		if (manager_impl->script_paths != NULL)
		{
			loader_manager_impl_script_paths_destroy(manager_impl->script_paths);
		}

		free(manager_impl);
	}
}
