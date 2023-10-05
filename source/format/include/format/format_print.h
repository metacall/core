/*
 *	Format Library by Parra Studios
 *	A cross-platform library for supporting formatted input / output.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
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

#include <stdarg.h>
#include <stdio.h>

/* -- Macros -- */

#if defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER < 1900)
	#ifndef snprintf
		#define snprintf _snprintf
	#endif
	#ifndef vsnprintf
		#define vsnprintf _vsnprintf
	#endif
#elif (defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER >= 1900)) || \
	defined(_BSD_SOURCE) || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 500) || \
	defined(_ISOC99_SOURCE) || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L) || \
	defined(__HAIKU__) || defined(__BEOS__) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)

	/* Supported */

#else
	/* TODO: Implement this only if needed */
	/*
	#ifndef snprintf
		#define snprintf snprintf_impl_c89
	#endif
	#ifndef vsnprintf
		#define vsnprintf vsnprintf_impl_c89
	#endif
	*/
#endif

/* -- Methods -- */

/*
FORMAT_API int snprintf_impl_c89(char *s, size_t n, const char *format, ...);

FORMAT_API int vsnprintf_impl_c89(char *s, size_t n, const char *format, va_list arg);
*/

#ifdef __cplusplus
}
#endif

#endif /* FORMAT_PRINT_H */
