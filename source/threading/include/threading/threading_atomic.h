/*
 *	Thrading Library by Parra Studios
 *	A threading library providing utilities for lock-free data structures and more.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef THREADING_ATOMIC_H
#define THREADING_ATOMIC_H 1

/* -- Headers -- */

#include <threading/threading_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#if defined(_WIN32) && defined(_MSC_VER)
	#if (_MSC_VER < 1930 || defined(__STDC_NO_ATOMICS__))
		/* Before Visual Studio 2022 atomics are not supported, use fallback solution */
		#include <threading/threading_atomic_win32.h>
		#define THREADING_ATOMIC 1
	#else
		#include <stdatomic.h>
		#define THREADING_ATOMIC 1
	#endif
#elif defined(__STDC_VERSION__)
	#if (__STDC_VERSION__ - 0L) >= 201112L
		/* C11 support */
		#if defined(__STDC_NO_ATOMICS__)
			#define THREADING_ATOMIC 0
			#error "Using C11 but atomics not supported, check the platform and implement support"
		#elif defined __has_include
			#if __has_include(<stdatomic.h>)
				#include <stdatomic.h>
				#define THREADING_ATOMIC 1
			#endif
		#else
			#include <stdatomic.h>
			#define THREADING_ATOMIC 1
		#endif
	#else
		#define THREADING_ATOMIC 0
		#error "C11 is not supported, check the platform and implement support"
	#endif
#else
	/* TODO: Unknown compiler and platform, check the platform and compiler, then implement support if needed */
	#define THREADING_ATOMIC 0
#endif

#if !defined(THREADING_ATOMIC) || (defined(THREADING_ATOMIC) && THREADING_ATOMIC == 0)
	#error "Thread atomic support not implemented."
#endif

#ifdef __cplusplus
}
#endif

#endif /* THREADING_ATOMIC_H */
