/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading python code at run-time into a process.
 *
 */

#include <metacall/metacall-version.h>

#include <py_loader/py_loader.h>

#include <stdio.h>

void py_loader_print_info()
{
	printf("Python Loader Plugin " METACALL_VERSION "\n");
	printf("Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n");

	#ifdef PY_LOADER_STATIC_DEFINE
		printf("Compiled as static library type\n");
	#else
		printf("Compiled as shared library type\n");
	#endif
}
