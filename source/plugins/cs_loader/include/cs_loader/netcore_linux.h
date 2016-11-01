#pragma once
#ifndef _NETCORELINUX_H_
#define _NETCORELINUX_H_

#include "netcore.h"
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <ostream>
#include <stdlib.h>
#include "dl/dynamicLinker.hpp"
#include <experimental/filesystem>
#include <functional>
#include <iostream>
#include <unistd.h>
#include "logger.h"
#define MAX_LONGPATH 255

typedef int (coreclrInitializeFunction)(
	const char* exePath,
	const char* appDomainFriendlyName,
	int propertyCount,
	const char** propertyKeys,
	const char** propertyValues,
	void** hostHandle,
	unsigned int* domainId);

// Prototype of the coreclr_shutdown function from the libcoreclr.so
typedef int (coreclrShutdownFunction)(
	void* hostHandle,
	unsigned int domainId);

// Prototype of the coreclr_execute_assembly function from the libcoreclr.so
typedef int (coreclrCreateDelegateFunction)(
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

	std::string coreClrDll = "libcoreclr.so";

	std::string absoluteLibPath;
	std::shared_ptr<dynamicLinker::dynamicLinker> dl;

	dynamicLinker::dynamicLinker::dlSymbol<coreclrInitializeFunction> * coreclr_initialize;
	dynamicLinker::dynamicLinker::dlSymbol<coreclrShutdownFunction> * coreclr_shutdown;
	dynamicLinker::dynamicLinker::dlSymbol<coreclrCreateDelegateFunction> * coreclr_create_delegate;

	std::string tpaList;

	bool ConfigAssemblyName();

	bool CreateHost();

	bool create_delegate(const CHARSTRING * delegateName, void** func);

	bool LoadMain();

	void AddFilesFromDirectoryToTpaList(std::string directory, std::string& tpaList) {

		for (auto& dirent : std::experimental::filesystem::directory_iterator(directory)) {
			std::string path = dirent.path();

			if (!path.compare(path.length() - 4, 4, ".dll")) {
				tpaList.append(path + ":");
			}
		}

	}

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