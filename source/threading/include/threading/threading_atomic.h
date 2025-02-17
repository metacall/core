/*
 *	Thrading Library by Parra Studios
 *	A threading library providing utilities for lock-free data structures and more.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
 
 /* Check for C11 support and atomics availability */
 #if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
	 #if defined(__STDC_NO_ATOMICS__)
		 /* C11 atomics not supported, use platform-specific implementation */
		 #if defined(_WIN32) && defined(_MSC_VER)
			 #include <threading/threading_atomic_win32.h>
		 #else
			 #error "Atomic operations not supported on this platform"
		 #endif
		 #define THREADING_ATOMIC 1
	 #else
		 /* C11 atomics are supported */
		 #include <stdatomic.h>
		 #include <threads.h>
		 #define THREADING_ATOMIC 1
	 #endif
 #elif defined(_WIN32) && defined(_MSC_VER)
	 /* Handle MSVC specific cases */
	 #if (_MSC_VER < 1930)
		 /* Before Visual Studio 2022, use Win32 atomic implementation */
		 #include <threading/threading_atomic_win32.h>
		 #define THREADING_ATOMIC 1
	 #else
		 /* Visual Studio 2022 and later */
		 #include <stdatomic.h>
		 #include <threads.h>
		 #define THREADING_ATOMIC 1
	 #endif
 #else
	 /* Platform not explicitly supported */
	 #error "Atomic operations not supported on this platform"
 #endif
 
 #ifndef THREADING_ATOMIC
	 #error "Thread atomic support not properly configured"
 #endif
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* THREADING_ATOMIC_H */
