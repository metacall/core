/*
 *	Format Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for supporting formatted input / output.
 *
 */

/* -- Headers -- */

#include <format/format_print.h>

/* -- Methods -- */

int snprintf_impl_c89(char * s, size_t n, const char * format, ...)
{
	va_list args;

	int result;

	/* TODO: Implement stack smashing prevention */
	(void)n;

	va_start(args, format);

	result = vsprintf(s, format, args);

	va_end(args);

	return result;
}

int vsnprintf_impl_c89(char * s, size_t n, const char * format, va_list arg)
{
	/* TODO: Implement stack smashing prevention */
	(void)n;

	return vsprintf(s, format, arg);
}
