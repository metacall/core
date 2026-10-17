/*
 *	Threading Library by Parra Studios
 *	A threading library providing utilities for lock-free data structures and more.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef THREADING_THREAD_H
#define THREADING_THREAD_H 1

/* -- Headers -- */

#include <threading/threading_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
	#include <windows.h>

typedef HANDLE threading_thread;
typedef DWORD threading_thread_id;
#elif (defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux) || defined(__gnu_linux__) || defined(__TOS_LINUX__)) || \
	defined(__FreeBSD__) || \
	defined(__NetBSD__) || \
	defined(__OpenBSD__) || \
	(defined(bsdi) || defined(__bsdi__)) || \
	defined(__DragonFly__) || \
	defined(__HAIKU__) || \
	((defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__))
	#include <pthread.h>

typedef pthread_t threading_thread;
typedef pthread_t threading_thread_id;
#else
	#error "Platform not supported for thread implementation"
#endif

/* -- Type Definitions -- */

typedef void *(*threading_thread_routine)(void *);

/* -- Methods -- */

/**
*  @brief
*    Create a new native OS thread and start executing @routine in it
*
*  @param[out] thread
*    Pointer to the thread handle to be initialized
*
*  @param[in] routine
*    Function to be executed by the new thread
*
*  @param[in] arg
*    Argument forwarded to @routine when the thread starts
*
*  @return
*    Zero on success, non-zero on failure
*/
THREADING_API int threading_thread_create(threading_thread *thread, threading_thread_routine routine, void *arg);

/**
*  @brief
*    Block the calling thread until @thread finishes execution
*
*  @param[in] thread
*    Pointer to the thread handle to be waited on
*
*  @param[out] result
*    If not null, receives the value returned by the thread routine.
*    On Windows the underlying native return value only carries an
*    exit code, so @result is always set to NULL there
*
*  @return
*    Zero on success, non-zero on failure
*/
THREADING_API int threading_thread_join(threading_thread *thread, void **result);

/**
*  @brief
*    Detach @thread so its resources are released automatically once it
*    finishes, without any other thread having to join it
*
*  @param[in] thread
*    Pointer to the thread handle to be detached
*
*  @return
*    Zero on success, non-zero on failure
*/
THREADING_API int threading_thread_detach(threading_thread *thread);

/**
*  @brief
*    Return the identifier of the calling thread
*
*  @return
*    Platform native id of the current thread
*/
THREADING_API threading_thread_id threading_thread_self(void);

/**
*  @brief
*    Release any platform resources still held by @thread after a
*    successful threading_thread_join. This is a no-op on platforms
*    (e.g POSIX) where join already reclaims the thread resources,
*    and must not be called after threading_thread_detach
*
*  @param[in] thread
*    Pointer to the thread handle to be destroyed
*/
THREADING_API void threading_thread_destroy(threading_thread *thread);

#ifdef __cplusplus
}
#endif

#endif /* THREADING_THREAD_H */
