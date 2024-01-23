/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <rb_loader/rb_loader.h>
#include <rb_loader/rb_loader_impl.h>

loader_impl_interface rb_loader_impl_interface_singleton(void)
{
	static struct loader_impl_interface_type loader_impl_interface_rb = {
		&rb_loader_impl_initialize,
		&rb_loader_impl_execution_path,
		&rb_loader_impl_load_from_file,
		&rb_loader_impl_load_from_memory,
		&rb_loader_impl_load_from_package,
		&rb_loader_impl_clear,
		&rb_loader_impl_discover,
		&rb_loader_impl_destroy
	};

	return &loader_impl_interface_rb;
}

const char *rb_loader_print_info(void)
{
	static const char rb_loader_info[] =
		"Ruby Loader Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef RB_LOADER_STATIC_DEFINE
		"Compiled as static library type\n"
#else
		"Compiled as shared library type\n"
#endif

		"\n";

	return rb_loader_info;
}
