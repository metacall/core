/*
 *	Loader Library by Parra Studios
 *	A plugin for loading net code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef _HOST_ENVIRONMENT_H_
#define _HOST_ENVIRONMENT_H_

#include <cs_loader/string_buffer.h>

#include <inc/palclr.h>
#include <pal/prebuilt/inc/mscoree.h>

#include <cs_loader/defs.h>

#include <functional>
#include <memory>

static const wchar_t *core_clr_dll = W("CoreCLR.dll");

class host_environment
{
private:
	wchar_t host_path[MAX_LONGPATH];

	// The path to the directory containing this module
	wchar_t host_directory_path[MAX_LONGPATH];

	// The name of this module, without the path
	wchar_t *host_exe_name;

	// The list of paths to the assemblies that will be trusted by CoreCLR
	string_buffer tpa_list;

	ICLRRuntimeHost2 *clr_runtime_host;

	HMODULE core_clr_module;

	HMODULE try_load_core_clr(const wchar_t *directory_path);

public:
	wchar_t core_clr_directory_path[MAX_LONGPATH];

	host_environment();
	host_environment(char *dotnet_root);
	~host_environment();

	bool tpa_list_contains_file(_In_z_ wchar_t *file_name_without_extension, _In_reads_(count_extensions) wchar_t **rg_tpa_extensions, int count_extensions);

	void remove_extension_and_ni(_In_z_ wchar_t *file_name);

	void add_files_from_directory_to_tpa_list(_In_z_ wchar_t *target_path, _In_reads_(count_extensions) wchar_t **rg_tpa_extensions, int count_extensions);

	// Returns the semicolon-separated list of paths to runtime dlls that are considered trusted.
	// On first call, scans the coreclr directory for dlls and adds them all to the list.
	const wchar_t *get_tpa_list();

	// Returns the path to the host module
	const wchar_t *get_host_path() const;

	// Returns the path to the host module
	const wchar_t *get_host_exe_name() const;

	// Returns the ICLRRuntimeHost2 instance, loading it from CoreCLR.dll if necessary, or nullptr on failure.
	ICLRRuntimeHost2 *get_clr_runtime_host();
};

#endif
