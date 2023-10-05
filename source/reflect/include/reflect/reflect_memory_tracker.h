/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#ifndef REFLECT_MEMORY_TRACKER_H
#define REFLECT_MEMORY_TRACKER_H 1

#include <reflect/reflect_api.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(REFLECT_MEMORY_TRACKER) && REFLECT_MEMORY_TRACKER == 1

	#include <threading/threading_atomic.h>

	#include <format/format_specifier.h>

	#include <stdio.h>

	#define reflect_memory_tracker(name) \
		static struct \
		{ \
			atomic_uintmax_t allocations; \
			atomic_uintmax_t deallocations; \
			atomic_uintmax_t increments; \
			atomic_uintmax_t decrements; \
		} name = { 0, 0, 0, 0 }

	#define reflect_memory_tracker_allocation(name) \
		atomic_fetch_add_explicit(&name.allocations, 1, memory_order_relaxed)

	#define reflect_memory_tracker_deallocation(name) \
		atomic_fetch_add_explicit(&name.deallocations, 1, memory_order_relaxed)

	#define reflect_memory_tracker_increment(name) \
		atomic_fetch_add_explicit(&name.increments, 1, memory_order_relaxed)

	#define reflect_memory_tracker_decrement(name) \
		atomic_fetch_add_explicit(&name.decrements, 1, memory_order_relaxed)

	#if !defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__)
		#define reflect_memory_tracker_print(name, title) \
			do \
			{ \
				printf("----------------- " title " -----------------\n"); \
				printf("Allocations: %" PRIuMAX "\n", atomic_load_explicit(&name.allocations, memory_order_relaxed)); \
				printf("Deallocations: %" PRIuMAX "\n", atomic_load_explicit(&name.deallocations, memory_order_relaxed)); \
				printf("Increments: %" PRIuMAX "\n", atomic_load_explicit(&name.increments, memory_order_relaxed)); \
				printf("Decrements: %" PRIuMAX "\n", atomic_load_explicit(&name.decrements, memory_order_relaxed)); \
				fflush(stdout); \
			} while (0)
	#else
		#define reflect_memory_tracker_print(name, title) \
			do \
			{ \
				uintmax_t allocations = atomic_load_explicit(&name.allocations, memory_order_relaxed); \
				uintmax_t deallocations = atomic_load_explicit(&name.deallocations, memory_order_relaxed); \
				uintmax_t increments = atomic_load_explicit(&name.increments, memory_order_relaxed); \
				uintmax_t decrements = atomic_load_explicit(&name.decrements, memory_order_relaxed); \
				/* This comparison is safe to be done like this because it is done once execution has finalized */ \
				if (allocations != deallocations || increments != decrements) \
				{ \
					printf("----------------- " title " -----------------\n"); \
					printf("Allocations: %" PRIuMAX "\n", allocations); \
					printf("Deallocations: %" PRIuMAX "\n", deallocations); \
					printf("Increments: %" PRIuMAX "\n", increments); \
					printf("Decrements: %" PRIuMAX "\n", decrements); \
					fflush(stdout); \
				} \
			} while (0)
	#endif
#else
	#define reflect_memory_tracker(name) \
		typedef char reflect_memory_tracker_disabled

	#define reflect_memory_tracker_allocation(name) \
		do \
		{ \
		} while (0)

	#define reflect_memory_tracker_deallocation(name) \
		do \
		{ \
		} while (0)

	#define reflect_memory_tracker_increment(name) \
		do \
		{ \
		} while (0)

	#define reflect_memory_tracker_decrement(name) \
		do \
		{ \
		} while (0)

	#define reflect_memory_tracker_print(name, title) \
		do \
		{ \
		} while (0)
#endif

void reflect_memory_tracker_debug(void);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_MEMORY_TRACKER_H */
