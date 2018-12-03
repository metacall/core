/*
 *	Format Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for supporting formatted input / output.
 *
 */

/* -- Headers -- */

#include <metacall/metacall_version.h>

#include <format/format.h>

/* -- Methods -- */

const char * format_print_info()
{
	static const char format_info[] =
		"Format Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef LOG_STATIC_DEFINE
			"Compiled as static library type\n"
		#else
			"Compiled as shared library type\n"
		#endif

		"\n";

	return format_info;
}
