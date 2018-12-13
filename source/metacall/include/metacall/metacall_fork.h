/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef METACALL_FORK_H
#define METACALL_FORK_H 1

/* -- Headers -- */

#include <metacall/metacall_api.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)

/* -- Headers -- */

#include <process.h>

/* -- Type Definitions -- */

typedef int metacall_pid;

#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)

/* -- Headers -- */

#include <sys/types.h>
#include <unistd.h>

/* -- Type Definitions -- */

typedef pid_t metacall_pid;

#else
#	error "Unknown metacall fork safety platform"
#endif

typedef int (*metacall_fork_callback_ptr)(metacall_pid, void *);

/* -- Methods -- */

/**
*  @brief
*    Initialize fork detours and allocate shared memory
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_fork_initialize(void);

/**
*  @brief
*    Set fork hook callback
*
*  @param[in] callback
*    Callback to be called when fork detour is executed
*/
METACALL_API void metacall_fork(metacall_fork_callback_ptr callback);

/**
*  @brief
*    Unregister fork detours and destroy shared memory
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_fork_destroy(void);

#ifdef __cplusplus
}
#endif

#endif  /* METACALL_FORK_H */
