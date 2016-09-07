/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#include <metacall/metacall-version.h>

#include <log/log.h>

#include <stdio.h>

void log_print_info()
{
	printf("Logger Library " METACALL_VERSION "\n");
	printf("Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n");

	#ifdef LOG_STATIC_DEFINE
		printf("Compiled as static library type\n");
	#else
		printf("Compiled as shared library type\n");
	#endif
}
