/*
 *	Loader Library by Parra Studios
 *	A plugin for loading net code at run-time into a process.
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

#include <cs_loader/netcore_linux.h>

#include <portability/portability_executable_path.h>

#include <log/log.h>

#include <limits.h>
#include <string.h>

netcore_linux::netcore_linux(char *dotnet_root, char *dotnet_loader_assembly_path) :
	netcore(dotnet_root, dotnet_loader_assembly_path), domainId(0)
{
	if (dotnet_root == NULL)
	{
		this->runtimePath.append(getenv("CORE_ROOT"));
	}
	else
	{
		if (dotnet_root[strlen(dotnet_root)] == '/')
		{
			this->runtimePath.append(dotnet_root);
		}
		else
		{
			char new_dotnet_root[0xff];
			strcpy(new_dotnet_root, dotnet_root);
			strcat(new_dotnet_root, "/");
			this->runtimePath.append(new_dotnet_root);
		}
	}

	if (getcwd(this->appPath, MAX_LONGPATH) == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "getcwd error");
	}
}

netcore_linux::~netcore_linux()
{
	this->stop();

	if (this->libHandle)
	{
		dynlink_unload(this->libHandle);
	}
}

bool netcore_linux::ConfigAssemblyName()
{
	std::string::size_type pos = std::string(this->dotnet_loader_assembly_path).find_last_of("\\/");

	std::string dotnet_loader_assembly_directory = std::string(this->dotnet_loader_assembly_path).substr(0, pos);

	// strcpy(this->appPath,dotnet_loader_assembly_directory.c_str());

	if (this->dotnet_loader_assembly_path == NULL)
	{
		this->managedAssemblyFullName.append(this->appPath);
		this->managedAssemblyFullName.append("/");
		this->managedAssemblyFullName.append(this->loader_dll);
	}
	else
	{
		if (this->dotnet_loader_assembly_path[0] == '/')
		{
			this->managedAssemblyFullName.append(this->dotnet_loader_assembly_path);
			AddFilesFromDirectoryToTpaList(dotnet_loader_assembly_directory, tpaList);
		}
		else
		{
			this->managedAssemblyFullName.append(this->appPath);
			this->managedAssemblyFullName.append("/");

			if (this->dotnet_loader_assembly_path[0] == '.')
			{
				string simpleName;
				simpleName.append(this->dotnet_loader_assembly_path + 2);
				this->managedAssemblyFullName.append(simpleName);
			}
			else
			{
				this->managedAssemblyFullName.append(this->dotnet_loader_assembly_path);
			}
		}
	}

	this->nativeDllSearchDirs.append(this->appPath);
	this->nativeDllSearchDirs.append(":");
	this->nativeDllSearchDirs.append(this->runtimePath);

	AddFilesFromDirectoryToTpaList(this->runtimePath, tpaList);

	log_write("metacall", LOG_LEVEL_DEBUG, "NetCore application absolute path: %s", this->appPath);

	/* TODO: Solve uninitialized strings */
	/*
	log_write("metacall", LOG_LEVEL_DEBUG, "absoluteRuntime: %s", this->runtimePath);
	log_write("metacall", LOG_LEVEL_DEBUG, "absoluteLoaderDll: %s", this->managedAssemblyFullName);
	*/

	return true;
}

bool netcore_linux::CreateHost()
{
	this->libHandle = dynlink_load(this->runtimePath.c_str(), this->coreClrLibName.c_str(), DYNLINK_FLAGS_BIND_NOW | DYNLINK_FLAGS_BIND_GLOBAL);

	if (this->libHandle == NULL)
	{
		return false;
	}

	dynlink_symbol_addr dynlink_coreclr_initialize;
	dynlink_symbol_addr dynlink_coreclr_shutdown;
	dynlink_symbol_addr dynlink_coreclr_create_delegate;

	dynlink_symbol(this->libHandle, "coreclr_initialize", &dynlink_coreclr_initialize);
	dynlink_symbol(this->libHandle, "coreclr_shutdown", &dynlink_coreclr_shutdown);
	dynlink_symbol(this->libHandle, "coreclr_create_delegate", &dynlink_coreclr_create_delegate);

	if (dynlink_coreclr_initialize == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "coreclr_initialize pointer not found");
		return false;
	}

	this->coreclr_initialize = (coreclrInitializeFunction *)dynlink_coreclr_initialize;
	this->coreclr_shutdown = (coreclrShutdownFunction *)dynlink_coreclr_shutdown;
	this->coreclr_create_delegate = (coreclrCreateDelegateFunction *)dynlink_coreclr_create_delegate;

	if (this->coreclr_initialize == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "coreclr_initialize fail");
		return false;
	}

	const char *propertyKeys[] = {
		"TRUSTED_PLATFORM_ASSEMBLIES",
		"APP_PATHS",
		"APP_NI_PATHS",
		"NATIVE_DLL_SEARCH_DIRECTORIES",
		"AppDomainCompatSwitch",
		"APP_CONTEXT_BASE_DIRECTORY"
	};

	const char *propertyValues[] = {
		tpaList.c_str(),
		appPath,
		appPath,
		nativeDllSearchDirs.c_str(),
		"UseLatestBehaviorWhenTFMNotSpecified",
		appPath
	};

	portability_executable_path_str exe_path_str = { 0 };
	portability_executable_path_length length = 0;

	if (portability_executable_path(exe_path_str, &length) != 0)
	{
		return false;
	}

	log_write("metacall", LOG_LEVEL_DEBUG, "coreclr_initialize working directory path (%s)", exe_path_str);

	// Initialize CoreCLR
	int status = (*this->coreclr_initialize)(
		exe_path_str,
		"metacall_cs_loader_container",
		sizeof(propertyKeys) / sizeof(propertyKeys[0]),
		propertyKeys,
		propertyValues,
		&hostHandle,
		&domainId);

	if (status < 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "coreclr_initialize status (0x%08x)", status);
		return false;
	}

	if (!this->create_delegates())
	{
		return false;
	}

	return true;
}

bool netcore_linux::LoadMain()
{
	return true;
}

bool netcore_linux::create_delegate(const CHARSTRING *delegateName, void **funcs)
{
	int status = -1;

	/* TODO: Implement exception handling with TRY_EX/CATCH_EX,
		STL exceptions are not working */
	try
	{
		// create delegate to our entry point
		status = (*coreclr_create_delegate)(
			hostHandle,
			domainId,
			this->assembly_name,
			this->class_name,
			delegateName,
			funcs);
	}
	catch (std::exception &ex)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "CreateDelegate exception (%s)", ex.what());
	}

	if (status < 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "CreateDelegate status (0x%08x)", status);
		return false;
	}

	return true;
}

bool netcore_linux::start()
{
	if (!ConfigAssemblyName())
	{
		return false;
	}

	if (!this->CreateHost())
	{
		return false;
	}

	if (!this->LoadMain())
	{
		return false;
	}

	return true;
}

void netcore_linux::stop()
{
	int status = (*this->coreclr_shutdown)(this->hostHandle, this->domainId);

	if (status != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Stop status (0x%08x)", status);
	}
}
