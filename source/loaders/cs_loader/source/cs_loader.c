/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading net code at run-time into a process.
 *
 */

#include <metacall/metacall_version.h>

#include <cs_loader/cs_loader.h>
#include <cs_loader/cs_loader_impl.h>
#include <loader/loader.h>
#include <loader/loader_impl.h>
#include <loader/loader_impl_interface.h>

loader_impl_interface cs_loader_impl_interface_singleton(void)
{
	static struct loader_impl_interface_type loader_impl_interface_cs = {
		&cs_loader_impl_initialize,
		&cs_loader_impl_execution_path,
		&cs_loader_impl_load_from_file,
		&cs_loader_impl_load_from_memory,
		&cs_loader_impl_load_from_package,
		&cs_loader_impl_clear,
		&cs_loader_impl_discover,
		&cs_loader_impl_destroy
	};

	return &loader_impl_interface_cs;
}

const char *cs_loader_print_info(void)
{
	static const char cs_loader_info[] =
		"Net Loader Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef CS_LOADER_STATIC_DEFINE
		"Compiled as static library type\n"
#else
		"Compiled as shared library type\n"
#endif

		"\n";

	return cs_loader_info;
}
