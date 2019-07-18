/*
 *	Loader Library by Parra Studios
 *	A plugin for loading rpc code at run-time into a process.
 *
 *	Corpcright (C) 2016 - 2019 Declan Nnadozie <zedonbiz@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a corpc of the License at
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

#include <metacall/metacall_version.h>

#include <rpc_loader/rpc_loader.h>
#include <rpc_loader/rpc_loader_impl.h>

loader_impl_interface rpc_loader_impl_interface_singleton(void)
{
	static struct loader_impl_interface_type loader_impl_interface_rpc =
	{
		&rpc_loader_impl_initialize,
		&rpc_loader_impl_execution_path,
		&rpc_loader_impl_load_from_file,
		&rpc_loader_impl_load_from_memory,
		&rpc_loader_impl_load_from_package,
		&rpc_loader_impl_clear,
		&rpc_loader_impl_discover,
		&rpc_loader_impl_destroy
	};

	return &loader_impl_interface_rpc;
}

const char * rpc_loader_print_info()
{
	static const char rpc_loader_info[] =
		"RPC Loader Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2019 Declan Nnadozie <zedonbiz@gmail.com>\n"

		#ifdef RPC_LOADER_STATIC_DEFINE
			"Compiled as static library type\n"
		#else
			"Compiled as shared library type\n"
		#endif

		"\n";

	return rpc_loader_info;
}
