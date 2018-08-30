/*
 *	Environment Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for supporting formatted input / output.
 *
 */

/* -- Headers -- */

#include <metacall/metacall_version.h>

#include <environment/environment.h>

/* -- Methods -- */

const char * environment_print_info()
{
	static const char environment_info[] =
		"Format Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef LOG_STATIC_DEFINE
			"Compiled as static library type\n"
		#else
			"Compiled as shared library type\n"
		#endif

		"\n";

	return environment_info;
}
