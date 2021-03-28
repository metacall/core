/*
 *	Loader Library by Parra Studios
 *	A plugin for loading mock code at run-time into a process.
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

#include <metacall/metacall_version.h>

#include <mock_loader/mock_loader.h>
#include <mock_loader/mock_loader_impl.h>

loader_impl_interface mock_loader_impl_interface_singleton()
{
	static struct loader_impl_interface_type loader_impl_interface_mock = {
		&mock_loader_impl_initialize,
		&mock_loader_impl_execution_path,
		&mock_loader_impl_load_from_file,
		&mock_loader_impl_load_from_memory,
		&mock_loader_impl_load_from_package,
		&mock_loader_impl_clear,
		&mock_loader_impl_discover,
		&mock_loader_impl_destroy
	};

	return &loader_impl_interface_mock;
}

const char *mock_loader_print_info()
{
	static const char mock_loader_info[] =
		"Mock Loader Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef MOCK_LOADER_STATIC_DEFINE
		"Compiled as static library type\n"
#else
		"Compiled as shared library type\n"
#endif

		"\n";

	return mock_loader_info;
}
