#include <call.h>
#include <stdlib.h>

void * metacall(char * func, ...)
{
	va_list ap;

	va_start(ap, func);

	// va_arg(ap, type);

	va_end(ap);

	return NULL;
}
