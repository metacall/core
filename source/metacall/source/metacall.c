#include <metacall/metacall-version.h>
#include <metacall/metacall.h>

#include <stdio.h>

void metacall_print_info()
{
	printf("MetaCall Library " METACALL_VERSION "\n");
	printf("Copyright (c) 2016 Vicente Eduardo Ferrer Garcia\n");

	#ifdef METACALL_STATIC_DEFINE
		printf("Compiled as static library type\n");
	#else
		printf("Compiled as shared library type\n");
	#endif
}
