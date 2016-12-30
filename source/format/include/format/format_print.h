/*
 *	Format Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for supporting formatted input / output.
 *
 */

#ifndef FORMAT_PRINT_H
#define FORMAT_PRINT_H 1

/* -- Headers -- */

#include <format/format_api.h>

#include <format/format_specifier.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdio.h>
#include <stdarg.h>

/* -- Macros -- */

#if defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER < 1900)
#	define snprintf _snprintf
#	define vsnprintf _vsnprintf
#elif (defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER >= 1900)) || \
	defined(_BSD_SOURCE) || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 500) || \
	defined(_ISOC99_SOURCE) || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L)

	/* Supported */

#else
#	define snprintf snprintf_c89_impl
#	define vsnprintf vsnprintf_c89_impl
#endif

/* -- Methods -- */

FORMAT_API int snprintf_c89_impl(char * s, size_t n, const char * format, ...);

FORMAT_API int vsnprintf_c89_impl(char * s, size_t n, const char * format, va_list arg);

#ifdef __cplusplus
}
#endif

#endif /* FORMAT_PRINT_H */
