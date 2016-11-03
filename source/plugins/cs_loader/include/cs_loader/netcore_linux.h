#pragma once
#ifndef _NETCORELINUX_H_
#define _NETCORELINUX_H_

#include <cs_loader/netcore.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <ostream>
#include <stdlib.h>
#include <dynlink/dynlink.h>
#include <iostream>

#include <cs_loader/logger.h>
#define MAX_LONGPATH 255

typedef int(*coreclrInitializeFunction)(
	const char* exePath,
	const char* appDomainFriendlyName,
	int propertyCount,
	const char** propertyKeys,
	const char** propertyValues,
	void** hostHandle,
	unsigned int* domainId);

// Prototype of the coreclr_shutdown function from the libcoreclr.so
typedef int(*coreclrShutdownFunction)(
	void* hostHandle,
	unsigned int domainId);

// Prototype of the coreclr_execute_assembly function from the libcoreclr.so
typedef int(*coreclrCreateDelegateFunction)(
	void* hostHandle,
	unsigned int domainId,
	const char* entryPointAssemblyName,
	const char* entryPointTypeName,
	const char* entryPointMethodName,
	void** delegatea);

using namespace std;
#define W(x) x

class netcore_linux : public netcore
{
private:
	void* hostHandle = NULL;
	logger log;
	std::string managedAssemblyFullName;
	char appPath[MAX_LONGPATH] = "";
	std::string runtimePath;
	char appNiPath[MAX_LONGPATH * 2] = "";
	std::string nativeDllSearchDirs;
	unsigned int domainId = 0;

	std::string coreClrDll = "coreclr";

	std::string absoluteLibPath;
	dynlink dl_handle;

	coreclrInitializeFunction   coreclr_initialize;
	coreclrShutdownFunction   coreclr_shutdown;
	coreclrCreateDelegateFunction  coreclr_create_delegate;

	std::string tpaList;

	bool ConfigAssemblyName();

	bool CreateHost();

	bool create_delegate(const CHARSTRING * delegateName, void** func);

	bool LoadMain();

	void add_files_from_directory_to_tpa_list(std::string directory, std::string& tpaList);

public:
	netcore_linux();
	~netcore_linux();

	bool start() {
		if (!ConfigAssemblyName()) {
			return false;
		}

		if (!this->CreateHost()) {
			return false;
		}
		if (!this->LoadMain()) {
			return false;
		}

		return true;
	}

	void stop();
};

#endif