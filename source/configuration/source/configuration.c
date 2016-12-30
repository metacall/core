/*
*	Configuration Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple configuration formats.
*
*/

/* -- Headers -- */

#include <metacall/metacall-version.h>

#include <configuration/configuration.h>

#include <log/log.h>

/* -- Member Data -- */

/* -- Methods -- */

const char * configuration_print_info()
{
	static const char configuration_info[] =
		"Configuration Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef CONFIGURATION_STATIC_DEFINE
			"Compiled as static library type"
		#else
			"Compiled as shared library type"
		#endif

		"\n";

	return configuration_info;
}
