/*
 *	Thrading Library by Parra Studios
 *	A threading library providing utilities for lock-free data structures and more.
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

#ifndef THREADING_MUTEX_H
#define THREADING_MUTEX_H 1

/* -- Headers -- */

#include <threading/threading_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Type Definitions -- */

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
	#include <windows.h>
typedef CRITICAL_SECTION threading_mutex_impl_type;
#elif (defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux) || defined(__gnu_linux__) || defined(__TOS_LINUX__)) || \
	defined(__FreeBSD__) ||                                                                                                                     \
	defined(__NetBSD__) ||                                                                                                                      \
	defined(__OpenBSD__) ||                                                                                                                     \
	(defined(bsdi) || defined(__bsdi__)) ||                                                                                                     \
	defined(__DragonFly__)
	#include <pthread.h>
typedef pthread_mutex_t threading_mutex_impl_type;
#elif (defined(__MACOS__) || defined(macintosh) || defined(Macintosh) || defined(__TOS_MACOS__)) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
	#include <os/lock.h>
typedef os_unfair_lock threading_mutex_impl_type;
#else
	#error "Platform not supported for mutex implementation"
#endif

#include <string.h>

/* -- Member Data -- */

struct threading_mutex_type
{
	threading_mutex_impl_type impl;
};

/* -- Type Definitions -- */

typedef struct threading_mutex_type *threading_mutex;

/* -- Methods -- */

int threading_mutex_initialize(threading_mutex m);

int threading_mutex_lock(threading_mutex m);

int threading_mutex_try_lock(threading_mutex m);

int threading_mutex_unlock(threading_mutex m);

int threading_mutex_destroy(threading_mutex m);

static inline int threading_mutex_store(threading_mutex m, void *dest, void *src, size_t size)
{
	if (threading_mutex_lock(m) != 0)
	{
		return 1;
	}

	memcpy(dest, src, size);

	return threading_mutex_unlock(m);
}

#ifdef __cplusplus
}
#endif

#endif /* THREADING_MUTEX_H */
