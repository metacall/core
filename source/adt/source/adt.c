#include <metacall/metacall-version.h>

#include <adt/adt.h>

#include <stdio.h>

void adt_print_info()
{
	printf("Abstract Data Type Library " METACALL_VERSION "\n");
	printf("Copyright (c) 2016 Vicente Eduardo Ferrer Garcia\n");

	#ifdef ADT_STATIC_DEFINE
		printf("Compiled as static library type\n");
	#else
		printf("Compiled as shared library type\n");
	#endif
}
