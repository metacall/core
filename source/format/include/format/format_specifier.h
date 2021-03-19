/*
 *	Format Library by Parra Studios
 *	A cross-platform library for supporting formatted input / output.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef FORMAT_SPECIFIER_H
#define FORMAT_SPECIFIER_H 1

/* -- Headers -- */

#include <format/format_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stddef.h>
#include <stdint.h>

#if !defined(_WIN32) &&                                                                     \
	(defined(unix) || defined(__unix__) || defined(__unix) ||                               \
		defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
		defined(__CYGWIN__) || defined(__CYGWIN32__) ||                                     \
		(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__))

	#include <unistd.h>

	#if defined(_POSIX_VERSION)
		#define FORMAT_POSIX 1 /* POSIX */
	#endif
#endif

#if defined(FORMAT_POSIX) && (defined(_INTTYPES_H) || defined(_INTTYPES_H_)) && !defined(PRId64)
	#error "inttypes.h already included without __STDC_FORMAT_MACROS previously defined."
#endif

#if defined(FORMAT_POSIX) && !defined(__STDC_FORMAT_MACROS)
	#define __STDC_FORMAT_MACROS 1
#endif

#include <inttypes.h>

/* -- Definitions -- */

#if defined(__LP64__) || defined(_LP64) ||                                                               \
	defined(_WIN64) ||                                                                                   \
	defined(alpha) || defined(__ALPHA) || defined(__alpha) || defined(__alpha__) || defined(_M_ALPHA) || \
	defined(_IA64) || defined(__IA64__) || defined(__ia64__) || defined(__ia64) || defined(_M_IA64) ||   \
	defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64) || defined(__amd64) ||                   \
	defined(__mips64) ||                                                                                 \
	defined(__powerpc64__) || defined(__ppc64__) || defined(_ARCH_PPC64) ||                              \
	defined(__arch64__) || defined(__sparcv9) || defined(__sparc_v9__) || defined(__sparc64__) ||        \
	defined(__s390__) || defined(__s390x__)

	#define FORMAT_64BIT 1 /* 64-bit */

#elif defined(_ILP32) || defined(__ILP32__) ||                                                                                                                       \
	defined(_WIN32) ||                                                                                                                                               \
	defined(__386__) || defined(i386) || defined(__i386) || defined(__i386__) || defined(__X86) || defined(_M_IX86) ||                                               \
	defined(__mips__) || defined(__mips) || defined(__MIPS__) || defined(_M_MRX000) ||                                                                               \
	defined(__hppa) || defined(__hppa__) ||                                                                                                                          \
	((defined(__PPC) || defined(__POWERPC__) || defined(__powerpc) || defined(__powerpc64__) || defined(__PPC__) ||                                                  \
		 defined(__powerpc__) || defined(__ppc__) || defined(__ppc) || defined(_ARCH_PPC) || defined(_M_PPC)) &&                                                     \
		!defined(__powerpc64__)) ||                                                                                                                                  \
	(defined(_ARCH_COM) && defined(_ARCH_PPC)) ||                                                                                                                    \
	((defined(__sparc__) || defined(__sparc) || defined(sparc)) && !(defined(__arch64__) || defined(__sparcv9) || defined(__sparc_v9__) || defined(__sparc64__))) || \
	defined(__arm__) || defined(__arm) || defined(ARM) || defined(_ARM) || defined(_ARM_) || defined(__ARM__) || defined(_M_ARM) ||                                  \
	defined(__m68k__) || defined(mc68000) || defined(_M_M68K) ||                                                                                                     \
	defined(__sh__) || defined(__sh) || defined(SHx) || defined(_SHX_) ||                                                                                            \
	defined(nios2) || defined(__nios2) || defined(__nios2__)

	#define FORMAT_32BIT 1 /* 32-bit */

#else
	#error "Unknown hardware processor architecture width."
#endif

#if defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER < 1900)
	#define FORMAT_PREFIX "I"
#elif defined(FORMAT_32BIT) ||                                                  \
	(defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER >= 1900)) ||             \
	defined(_BSD_SOURCE) || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 500) || \
	defined(_ISOC99_SOURCE) || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L)
	#define FORMAT_PREFIX "z"
#elif defined(FORMAT_64BIT)
	#if (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
		#define FORMAT_PREFIX "l"
	#else
		#define FORMAT_PREFIX
	#endif
#endif

#if !defined(PRIdS)
	#define PRIdS FORMAT_PREFIX "d"
#endif

#if !defined(PRIxS)
	#define PRIxS FORMAT_PREFIX "x"
#endif

#if !defined(PRIuS)
	#define PRIuS FORMAT_PREFIX "u"
#endif

#if !defined(PRIXS)
	#define PRIXS FORMAT_PREFIX "X"
#endif

#if !defined(PRIoS)
	#define PRIoS FORMAT_PREFIX "o"
#endif

#if defined(PRId64)
	#if defined(_WIN32)
		#define WPRId64 L"I64d"
	#else
		#define WPRId64 PRId64
	#endif
#endif

#if defined(PRIx64)
	#if defined(_WIN32)
		#define WPRIx64 L"I64x"
	#else
		#define WPRIx64 PRIx64
	#endif
#endif

#if defined(PRIu64)
	#if defined(_WIN32)
		#define WPRIu64 L"I64u"
	#else
		#define WPRIu64 PRIu64
	#endif
#endif

#if defined(PRIX64)
	#if defined(_WIN32)
		#define WPRIX64 L"I64X"
	#else
		#define WPRIX64 PRIX64
	#endif
#endif

#if defined(PRIo64)
	#if defined(_WIN32)
		#define WPRIo64 L"I64o"
	#else
		#define WPRIo64 PRIo64
	#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* FORMAT_SPECIFIER_H */
