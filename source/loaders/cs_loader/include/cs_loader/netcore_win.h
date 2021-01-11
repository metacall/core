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

#ifndef _NETCORE_WIN_H_
#define _NETCORE_WIN_H_
#include "defs.h"
#include "netcore.h"

typedef class host_environment;
typedef class ICLRRuntimeHost2;

class netcore_win : public netcore
{
private:
	host_environment * core_environment;
	ICLRRuntimeHost2 * host;
	wchar_t managedAssemblyFullName[MAX_LONGPATH] = W("");
	wchar_t appPath[MAX_LONGPATH] = W("");
	wchar_t appNiPath[MAX_LONGPATH * 2] = W("");
	wchar_t nativeDllSearchDirs[MAX_LONGPATH * 3];
	unsigned long domain_id;

	bool config_assembly_name();

	bool create_host();

	bool create_delegate(const wchar_t * delegate_name, void** func);

	bool load_main();

public:
	netcore_win(char * dotnet_root,char * dotnet_loader_assembly_path);
	~netcore_win();

	bool start();

	void stop();
};


#endif
