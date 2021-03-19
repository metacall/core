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

#include <cs_loader/simple_netcore.h>

#include <cs_loader/netcore.h>
#if defined(__linux) | defined(linux)
	#include <cs_loader/netcore_linux.h>
#else
	#include <cs_loader/netcore_win.h>
#endif

netcore_handle simple_netcore_create(char *dotnet_root, char *dotnet_loader_assembly_path)
{
#if defined(__linux) | defined(linux)
	netcore_linux *netcore_impl = new netcore_linux(dotnet_root, dotnet_loader_assembly_path);
#else
	netcore_win *netcore_impl = new netcore_win(dotnet_root, dotnet_loader_assembly_path);
#endif

	bool result = netcore_impl->start();

	if (result == false)
	{
		delete netcore_impl;

		return (netcore_handle)NULL;
	}

	return (netcore_handle)netcore_impl;
}

reflect_function *simple_netcore_get_functions(netcore_handle handle, int *count)
{
	netcore *core = (netcore *)handle;

	return core->get_functions(count);
}

void simple_netcore_destroy(netcore_handle handle)
{
	netcore *core = (netcore *)handle;
	delete core;
}

int simple_netcore_load_script_from_files(netcore_handle handle, char *files[MAX_FILES], size_t size)
{
	netcore *core = (netcore *)handle;

	return core->load_files(files, size) == true ? 0 : 1;
}

int simple_netcore_load_script_from_assembly(netcore_handle handle, char *file)
{
	netcore *core = (netcore *)handle;

	return core->load_assembly(file) == true ? 0 : 1;
}

int simple_netcore_load_script_from_memory(netcore_handle handle, const char *buffer, size_t size)
{
	netcore *core = (netcore *)handle;

	(void)size;

	return core->load_source((char *)buffer) == true ? 0 : 1;
}

execution_result *simple_netcore_invoke(netcore_handle handle, const char *func)
{
	netcore *core = (netcore *)handle;

	return core->execute((char *)func);
}

execution_result *simple_netcore_invoke_with_params(netcore_handle handle, const char *func, parameters *params)
{
	netcore *core = (netcore *)handle;

	return core->execute_with_params((char *)func, params);
}

void simple_netcore_destroy_execution_result(netcore_handle handle, execution_result *er)
{
	netcore *core = (netcore *)handle;

	core->destroy_execution_result(er);
}
