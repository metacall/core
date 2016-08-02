/*
*	Loader Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A plugin for loading mock code at run-time into a process.
*
*/

#include <mock_loader/mock_loader_print.h>

#include <metacall/metacall-version.h>

void mock_loader_print_info()
{
	printf("Mock Loader Plugin " METACALL_VERSION "\n");
	printf("Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n");

	#ifdef MOCK_LOADER_STATIC_DEFINE
		printf("Compiled as static library type\n");
	#else
		printf("Compiled as shared library type\n");
	#endif
}
