#pragma once
#ifndef _HOST_ENVIRONMENT_H_
#define _HOST_ENVIRONMENT_H_

#include <stdio.h>
#include <pal/prebuilt/inc/mscoree.h>
#include <cs_loader/logger.h>
#include <inc/palclr.h>

#include <functional>
#include <iostream>
#include <istream>
#include <sstream>
#include <fstream>
#include <memory>
#include <cs_loader/string_buffer.h>

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

	ICLRRuntimeHost2* clr_runtime_host;

	HMODULE core_clr_module;

	logger *log;

	HMODULE try_load_core_clr(const wchar_t* directory_path);

public:

	wchar_t core_clr_directory_path[MAX_LONGPATH];

	host_environment();
	host_environment(logger *logger);
	~host_environment();

	bool tpa_list_contains_file(_In_z_ wchar_t* file_name_without_extension, _In_reads_(count_extensions) wchar_t** rg_tpa_extensions, int count_extensions);

	void remove_extension_and_ni(_In_z_ wchar_t* file_name);

	void add_files_from_directory_to_tpa_list(_In_z_ wchar_t* target_path, _In_reads_(count_extensions) wchar_t** rg_tpa_extensions, int count_extensions);

	// Returns the semicolon-separated list of paths to runtime dlls that are considered trusted.
	// On first call, scans the coreclr directory for dlls and adds them all to the list.
	const wchar_t * get_tpa_list();

	// Returns the path to the host module
	const wchar_t * get_host_path() const;

	// Returns the path to the host module
	const wchar_t * get_host_exe_name() const;

	// Returns the ICLRRuntimeHost2 instance, loading it from CoreCLR.dll if necessary, or nullptr on failure.
	ICLRRuntimeHost2* get_clr_runtime_host();
};

#endif