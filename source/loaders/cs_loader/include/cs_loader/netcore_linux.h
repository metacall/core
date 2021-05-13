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

#ifndef _NETCORELINUX_H_
#define _NETCORELINUX_H_

#include <cs_loader/netcore.h>

#include <dynlink/dynlink.h>

#include <experimental/filesystem>

#include <functional>

#include <unistd.h>

#define MAX_LONGPATH 255

// Prototype of the coreclr_initialize function from the libcoreclr.so
typedef int(coreclrInitializeFunction)(
	const char *exePath,
	const char *appDomainFriendlyName,
	int propertyCount,
	const char **propertyKeys,
	const char **propertyValues,
	void **hostHandle,
	unsigned int *domainId);

// Prototype of the coreclr_shutdown function from the libcoreclr.so
typedef int(coreclrShutdownFunction)(
	void *hostHandle,
	unsigned int domainId);

// Prototype of the coreclr_execute_assembly function from the libcoreclr.so
typedef int(coreclrCreateDelegateFunction)(
	void *hostHandle,
	unsigned int domainId,
	const char *entryPointAssemblyName,
	const char *entryPointTypeName,
	const char *entryPointMethodName,
	void **delegatea);

using namespace std;

#ifdef W
	#undef W
#endif

#define W(x) x

class netcore_linux : public netcore
{
private:
	void *hostHandle = NULL;
	std::string managedAssemblyFullName;
	char appPath[MAX_LONGPATH] = "";
	std::string runtimePath;
	char appNiPath[MAX_LONGPATH * 2] = "";
	std::string nativeDllSearchDirs;
	unsigned int domainId = 0;

	std::string coreClrLibName = "coreclr";

	std::string absoluteLibPath;

	coreclrInitializeFunction *coreclr_initialize;
	coreclrShutdownFunction *coreclr_shutdown;
	coreclrCreateDelegateFunction *coreclr_create_delegate;

	std::string tpaList;

 	dynlink libHandle;

	bool ConfigAssemblyName();

	bool CreateHost();

	bool create_delegate(const CHARSTRING *delegateName, void **func);

	bool LoadMain();

	void AddFilesFromDirectoryToTpaList(std::string directory, std::string &tpaList)
	{
		for (auto &dirent : std::experimental::filesystem::directory_iterator(directory))
		{
			std::string path = dirent.path();

			if (!path.compare(path.length() - 4, 4, ".dll"))
			{
				tpaList.append(path + ":");
			}
		}
	}

public:
	netcore_linux(char *dotnet_root, char *dotnet_loader_assembly_path);

	~netcore_linux();

	bool start();

	void stop();
};

#endif
