/*
 *	Loader Library by Parra Studios
 *	A plugin for loading net code at run-time into a process.
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

#ifndef _SIMPLE_NETCORE_H_
#define _SIMPLE_NETCORE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <cs_loader/defs.h>
#include <stdlib.h>

#define MAX_FILES 0xFF

struct netcore_handle_type;
typedef struct netcore_handle_type *netcore_handle;

typedef char source_file[512];

netcore_handle simple_netcore_create(char *dotnet_root, char *dotnet_loader_assembly_path);

reflect_function *simple_netcore_get_functions(netcore_handle, int *);

int simple_netcore_load_script_from_files(netcore_handle handle, char *files[MAX_FILES], size_t size);

int simple_netcore_load_script_from_assembly(netcore_handle handle, char *file);

int simple_netcore_load_script_from_memory(netcore_handle handle, const char *buffer, size_t size);

execution_result *simple_netcore_invoke(netcore_handle, const char *);

void simple_netcore_destroy(netcore_handle);

execution_result *simple_netcore_invoke_with_params(netcore_handle handle, const char *func, parameters *params);

void simple_netcore_destroy_execution_result(netcore_handle handle, execution_result *er);

#ifdef __cplusplus
}
#endif
#endif
