/*
 *	Preprocessor Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic header only preprocessor metaprogramming library.
 *
 */

/* -- Headers -- */

#include <metacall/metacall-version.h>

#include <preprocessor/preprocessor.h>

/* -- Methods -- */

const char * preprocessor_print_info()
{
	static const char preprocessor_info[] =
		"Preprocessor Library " METACALL_VERSION "\n"
		"Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef PREPROCSSOR_STATIC_DEFINE
			"Compiled as static library type"
		#else
			"Compiled as shared library type"
		#endif

		"\n";

	return preprocessor_info;
}
