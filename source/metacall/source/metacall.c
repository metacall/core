/************************************************************************/
/*	MetaCall Library by Parra Studios									*/
/*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																		*/
/*	A library for providing a foreign function interface calls.			*/
/*																		*/
/************************************************************************/

#include <metacall/metacall-version.h>

#include <metacall/metacall.h>

#include <stdio.h>

void metacall_print_info()
{
	printf("MetaCall Library " METACALL_VERSION "\n");
	printf("Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n");

	#ifdef METACALL_STATIC_DEFINE
		printf("Compiled as static library type\n");
	#else
		printf("Compiled as shared library type\n");
	#endif
}
