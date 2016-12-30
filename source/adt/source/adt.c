/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

#include <metacall/metacall-version.h>

#include <adt/adt.h>

const char * adt_print_info()
{
	static const char adt_info[] =
		"Abstract Data Type Library " METACALL_VERSION "\n"
		"Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef ADT_STATIC_DEFINE
			"Compiled as static library type"
		#else
			"Compiled as shared library type"
		#endif

		"\n";

	return adt_info;
}
