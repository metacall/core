/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading file code at run-time into a process.
 *
 */

#include <metacall/metacall-version.h>

#include <file_loader/file_loader.h>
#include <file_loader/file_loader_impl.h>

loader_impl_interface file_loader_impl_interface_singleton()
{
	static struct loader_impl_interface_type loader_impl_interface_file =
	{
		&file_loader_impl_initialize,
		&file_loader_impl_execution_path,
		&file_loader_impl_load_from_file,
		&file_loader_impl_load_from_memory,
		&file_loader_impl_load_from_package,
		&file_loader_impl_clear,
		&file_loader_impl_discover,
		&file_loader_impl_destroy
	};

	return &loader_impl_interface_file;
}

const char * file_loader_print_info()
{
	static const char file_loader_info[] =
		"File Loader Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef FILE_LOADER_STATIC_DEFINE
			"Compiled as static library type\n"
		#else
			"Compiled as shared library type\n"
		#endif

		"\n";

	return file_loader_info;
}
