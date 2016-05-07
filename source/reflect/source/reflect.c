/************************************************************************/
/*	Reflect Library by Parra Studios									*/
/*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																		*/
/*	A library for provide reflection and metadata representation.		*/
/*																		*/
/************************************************************************/

#include <metacall/metacall-version.h>

#include <reflect/reflect.h>

#include <stdio.h>

void reflect_print_info(void)
{
	printf("Reflect Library " METACALL_VERSION "\n");
	printf("Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n");

	#ifdef METACALL_STATIC_DEFINE
		printf("Compiled as static library type\n");
	#else
		printf("Compiled as shared library type\n");
	#endif
}
