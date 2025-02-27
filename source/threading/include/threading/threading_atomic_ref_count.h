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

#ifndef THREADING_ATOMIC_REF_COUNT_H
#define THREADING_ATOMIC_REF_COUNT_H 1

/* -- Headers -- */

#include <threading/threading_api.h>

#include <threading/threading_atomic.h>

#if defined(__THREAD_SANITIZER__)
	#include <threading/threading_mutex.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdint.h>

/* -- Definitions -- */

#define THREADING_ATOMIC_REF_COUNT_MAX UINTMAX_MAX
#define THREADING_ATOMIC_REF_COUNT_MIN 0

/* -- Member Data -- */

struct threading_atomic_ref_count_type
{
#if defined(__THREAD_SANITIZER__)
	uintmax_t count;
	threading_mutex_type m;
#else
	atomic_uintmax_t count;
#endif
};

/* -- Type Definitions -- */

typedef struct threading_atomic_ref_count_type *threading_atomic_ref_count;

/* -- Methods -- */

static inline void threading_atomic_ref_count_store(threading_atomic_ref_count ref, uintmax_t v)
{
#if defined(__THREAD_SANITIZER__)
	threading_mutex_store(&ref->m, &ref->count, &v, sizeof(uintmax_t));
#else
	atomic_store(&ref->count, v);
#endif
}

static inline void threading_atomic_ref_count_initialize(threading_atomic_ref_count ref)
{
#if defined(__THREAD_SANITIZER__)
	uintmax_t init = THREADING_ATOMIC_REF_COUNT_MIN;

	threading_mutex_initialize(&ref->m);

	threading_mutex_store(&ref->m, &ref->count, &init, sizeof(uintmax_t));
#else
	threading_atomic_ref_count_store(ref, THREADING_ATOMIC_REF_COUNT_MIN);
#endif
}

static inline uintmax_t threading_atomic_ref_count_load(threading_atomic_ref_count ref)
{
#if defined(__THREAD_SANITIZER__)
	uintmax_t result = 0;

	threading_mutex_store(&ref->m, &result, &ref->count, sizeof(uintmax_t));

	return result;
#else
	return atomic_load_explicit(&ref->count, memory_order_relaxed);
#endif
}

static inline int threading_atomic_ref_count_increment(threading_atomic_ref_count ref)
{
#if defined(__THREAD_SANITIZER__)
	threading_mutex_lock(&ref->m);
	{
		++ref->count;
	}
	threading_mutex_unlock(&ref->m);
#else
	if (atomic_load_explicit(&ref->count, memory_order_relaxed) == THREADING_ATOMIC_REF_COUNT_MAX)
	{
		return 1;
	}

	atomic_fetch_add_explicit(&ref->count, 1, memory_order_relaxed);
#endif

	return 0;
}

static inline int threading_atomic_ref_count_decrement(threading_atomic_ref_count ref)
{
#if defined(__THREAD_SANITIZER__)
	threading_mutex_lock(&ref->m);
	{
		--ref->count;
	}
	threading_mutex_unlock(&ref->m);
#else
	if (atomic_load_explicit(&ref->count, memory_order_relaxed) == THREADING_ATOMIC_REF_COUNT_MIN)
	{
		return 1;
	}

	uintmax_t old_ref_count = atomic_fetch_sub_explicit(&ref->count, 1, memory_order_release);

	if (old_ref_count == THREADING_ATOMIC_REF_COUNT_MIN + 1)
	{
		atomic_thread_fence(memory_order_acquire);
	}
#endif

	return 0;
}

static inline void threading_atomic_ref_count_destroy(threading_atomic_ref_count ref)
{
#if defined(__THREAD_SANITIZER__)
	threading_mutex_destroy(&ref->m);
#else
	(void)ref;
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* THREADING_ATOMIC_REF_COUNT_H */
