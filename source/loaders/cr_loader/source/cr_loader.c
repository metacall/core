/*
 *	Loader Library by Parra Studios
 *	A plugin for loading crystal code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <metacall/metacall_version.h>

#include <cr_loader/cr_loader.h>
#include <cr_loader/cr_loader_impl.h>

loader_impl_interface cr_loader_impl_interface_singleton()
{
	static struct loader_impl_interface_type loader_impl_interface_cr =
	{
		&cr_loader_impl_initialize,
		&cr_loader_impl_execution_path,
		&cr_loader_impl_load_from_cr,
		&cr_loader_impl_load_from_memory,
		&cr_loader_impl_load_from_package,
		&cr_loader_impl_clear,
		&cr_loader_impl_discover,
		&cr_loader_impl_destroy
	};

	return &loader_impl_interface_cr;
}

const char * cr_loader_print_info()
{
	static const char cr_loader_info[] =
		"Crystal Loader Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef CR_LOADER_STATIC_DEFINE
			"Compiled as static library type\n"
		#else
			"Compiled as shared library type\n"
		#endif

		"\n";

	return cr_loader_info;
}
