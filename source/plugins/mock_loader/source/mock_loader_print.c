/*
*	Loader Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A plugin for loading mock code at run-time into a process.
*
*/

#include <mock_loader/mock_loader_print.h>

#include <metacall/metacall-version.h>

const char * mock_loader_print_info()
{
	static const char mock_loader_info[] =
		"Mock Loader Plugin " METACALL_VERSION "\n"
		"Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef MOCK_LOADER_STATIC_DEFINE
			"Compiled as static library type\n"
		#else
			"Compiled as shared library type\n"
		#endif

		"\n";

	return mock_loader_info;
}
