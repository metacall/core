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

static const char *core_clr_dll = "CoreCLR.dll";

class host_environment
{
private:
	char host_path[MAX_LONGPATH];

	// The path to the directory containing this module
	char host_directory_path[MAX_LONGPATH] = "";

	// The name of this module, without the path
	char *host_exe_name;

	// The list of paths to the assemblies that will be trusted by CoreCLR
	string_buffer tpa_list;

	ICLRRuntimeHost2* clr_runtime_host;

	HMODULE core_clr_module;

	logger *log;

	HMODULE try_load_core_clr(const char* directory_path);

public:

	char core_clr_directory_path[MAX_LONGPATH];

	host_environment();
	host_environment(logger *logger);
	~host_environment();

	bool tpa_list_contains_file(_In_z_ char* file_name_without_extension, _In_reads_(count_extensions) char** rg_tpa_extensions, int count_extensions);

	void remove_extension_and_ni(_In_z_ char* file_name);

	void add_files_from_directory_to_tpa_list(_In_z_ char* target_path, _In_reads_(count_extensions) char** rg_tpa_extensions, int count_extensions);

	// Returns the semicolon-separated list of paths to runtime dlls that are considered trusted.
	// On first call, scans the coreclr directory for dlls and adds them all to the list.
	const char * get_tpa_list();

	// Returns the path to the host module
	const char * get_host_path() const;

	// Returns the path to the host module
	const char * get_host_exe_name() const;

	// Returns the ICLRRuntimeHost2 instance, loading it from CoreCLR.dll if necessary, or nullptr on failure.
	ICLRRuntimeHost2* get_clr_runtime_host();
};

#endif