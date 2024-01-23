/*
 *	Loader Library by Parra Studios
 *	A plugin for loading nodejs code at run-time into a process.
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

#ifndef NODE_LOADER_BOOTSTRAP_H
#define NODE_LOADER_BOOTSTRAP_H 1

#include <configuration/configuration.h>

#include <reflect/reflect_value_type.h>

#include <loader/loader_naming.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>

inline int node_loader_impl_bootstrap_path(const char file[], size_t file_length, configuration config, loader_path path, size_t *size)
{
	size_t path_size = 0;
	const char *load_library_path = value_to_string(configuration_value(config, "loader_library_path"));
	size_t load_library_path_length = 0;

	if (load_library_path == NULL)
	{
		*size = 0;
		return 1;
	}

	load_library_path_length = strnlen(load_library_path, LOADER_PATH_SIZE);

	strncpy(path, load_library_path, load_library_path_length);

	if (path[load_library_path_length - 1] != '/' && path[load_library_path_length - 1] != '\\')
	{
#if defined(WIN32) || defined(_WIN32)
		path[load_library_path_length] = '\\';
#else
		path[load_library_path_length] = '/';
#endif

		++load_library_path_length;
	}

	/* Detect if another bootstrap script has been defined in the configuration */
	value bootstrap_value = configuration_value(config, "bootstrap_script");

	if (bootstrap_value != NULL)
	{
		/* Load bootstrap script defined in the configuration */
		const char *bootstrap_script = value_to_string(bootstrap_value);
		size_t bootstrap_script_length = strnlen(bootstrap_script, LOADER_PATH_SIZE);

		strncpy(&path[load_library_path_length], bootstrap_script, bootstrap_script_length);

		path_size = load_library_path_length + bootstrap_script_length + 1;

		path[path_size - 1] = '\0';
	}
	else
	{
		/* Load default script name */
		strncpy(&path[load_library_path_length], file, file_length + 1);

		path_size = load_library_path_length + file_length + 1;

		path[path_size - 1] = '\0';
	}

	*size = path_size;

	return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* NODE_LOADER_BOOTSTRAP_H */
