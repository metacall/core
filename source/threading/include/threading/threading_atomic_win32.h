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

#ifndef THREADING_ATOMIC_WIN32_H
#define THREADING_ATOMIC_WIN32_H 1

/* -- Headers -- */

#include <threading/threading_api.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef NOMINMAX
	#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef bool volatile atomic_bool;
typedef char volatile atomic_char;
typedef signed char volatile atomic_schar;
typedef unsigned char volatile atomic_uchar;
typedef short volatile atomic_short;
typedef unsigned short volatile atomic_ushort;
typedef int volatile atomic_int;
typedef unsigned int volatile atomic_uint;
typedef long volatile atomic_long;
typedef unsigned long volatile atomic_ulong;
typedef long long volatile atomic_llong;
typedef unsigned long long volatile atomic_ullong;
typedef size_t volatile atomic_size_t;
typedef ptrdiff_t volatile atomic_ptrdiff_t;
typedef intmax_t volatile atomic_intmax_t;
typedef uintmax_t volatile atomic_uintmax_t;
typedef intptr_t volatile atomic_intptr_t;
typedef uintptr_t volatile atomic_uintptr_t;
typedef uint8_t volatile atomic_uint8_t;
typedef uint16_t volatile atomic_uint16_t;
typedef uint32_t volatile atomic_uint32_t;
typedef uint64_t volatile atomic_uint64_t;
typedef int8_t volatile atomic_int8_t;
typedef int16_t volatile atomic_int16_t;
typedef int32_t volatile atomic_int32_t;
typedef int64_t volatile atomic_int64_t;

typedef enum memory_order
{
	memory_order_relaxed,
	memory_order_consume,
	memory_order_acquire,
	memory_order_release,
	memory_order_acq_rel,
	memory_order_seq_cst
} memory_order;

typedef LONG volatile atomic_flag;

#if _MSC_VER > 1800 && defined(_M_IX86)
	#define _InterlockedExchangeAdd64 _InlineInterlockedExchangeAdd64
	#define _InterlockedExchange64	  _InlineInterlockedExchange64
#endif

#define ATOMIC_BOOL_LOCK_FREE	  2
#define ATOMIC_CHAR_LOCK_FREE	  2
#define ATOMIC_CHAR16_T_LOCK_FREE 2
#define ATOMIC_CHAR32_T_LOCK_FREE 2
#define ATOMIC_WCHAR_T_LOCK_FREE  2
#define ATOMIC_SHORT_LOCK_FREE	  2
#define ATOMIC_INT_LOCK_FREE	  2
#define ATOMIC_LONG_LOCK_FREE	  2
#define ATOMIC_LLONG_LOCK_FREE	  2
#define ATOMIC_POINTER_LOCK_FREE  2

#define ATOMIC_FLAG_INIT	   0
#define ATOMIC_VAR_INIT(value) (value) /* TODO: Deprecate for C17, remove in C23 */

#define atomic_flag_test_and_set(obj) \
	(bool)_interlockedbittestandset((LONG *)obj, (LONG)0)

#define atomic_flag_test_and_set_explicit(obj, order) \
	atomic_flag_test_and_set(obj)

#define atomic_flag_clear(obj) \
	(bool)_interlockedbittestandreset((LONG *)obj, (LONG)0)

#define atomic_flag_clear_explicit(obj, order) \
	atomic_flag_clear(obj)

#define atomic_init(obj, desired) (*(obj) = (desired), (void)0)

#define __atomic_is_lock_free_power_of_2(x) ((x) && !((x) & ((x)-1)))

#define atomic_is_lock_free(obj) \
	(sizeof(obj) <= 8 && __atomic_is_lock_free_power_of_2(sizeof(obj)))

#undef __atomic_is_lock_free_power_of_2

inline bool atomic_store_explicit8(CHAR volatile *obj, CHAR desired, memory_order order)
{
	if (order == memory_order_seq_cst)
	{
		_InterlockedExchange8(obj, desired);
	}
	else
	{
		atomic_init(obj, desired);
	}
}

inline bool atomic_store_explicit16(SHORT volatile *obj, SHORT desired, memory_order order)
{
	if (order == memory_order_seq_cst)
	{
		_InterlockedExchange16(obj, desired);
	}
	else
	{
		atomic_init(obj, desired);
	}
}

inline bool atomic_store_explicit32(LONG volatile *obj, LONG desired, memory_order order)
{
	if (order == memory_order_seq_cst)
	{
		_InterlockedExchange(obj, desired);
	}
	else
	{
		atomic_init(obj, desired);
	}
}

inline bool atomic_store_explicit64(LONG64 volatile *obj, LONG64 desired, memory_order order)
{
	if (order == memory_order_seq_cst)
#ifdef _M_IX86
	{
		_InterlockedExchangeNoFence64(obj, desired);
	}
	else
	{
		_InterlockedExchange64(obj, desired);
	}
#else
	{
		_InterlockedExchange64(obj, desired);
	}
	else
	{
		atomic_init(obj, desired);
	}
#endif
}

#define atomic_store(obj, desired) \
	atomic_store_explicit(obj, desired, memory_order_seq_cst)

#define atomic_store_explicit(obj, desired, order)                                       \
	do                                                                                   \
	{                                                                                    \
		if (sizeof *(obj) == 1)                                                          \
		{                                                                                \
			atomic_store_explicit8((CHAR volatile *)(obj), (CHAR)(desired), order);      \
		}                                                                                \
		else if (sizeof *(obj) == 2)                                                     \
		{                                                                                \
			atomic_store_explicit16((SHORT volatile *)(obj), (SHORT)(desired), order);   \
		}                                                                                \
		else if (sizeof *(obj) == 4)                                                     \
		{                                                                                \
			atomic_store_explicit32((LONG volatile *)(obj), (LONG)(desired), order);     \
		}                                                                                \
		else if (sizeof *(obj) == 8)                                                     \
		{                                                                                \
			atomic_store_explicit64((LONG64 volatile *)(obj), (LONG64)(desired), order); \
		}                                                                                \
		else                                                                             \
		{                                                                                \
			abort();                                                                     \
		}                                                                                \
	} while (0)

#define atomic_load(obj) \
	atomic_fetch_or((obj), 0)

#define atomic_load_explicit(obj, order) \
	atomic_fetch_or_explicit((obj), 0, order)

#define atomic_exchange(obj, desired) \
	atomic_exchange_explicit(obj, desired, memory_order_seq_cst)

#define atomic_exchange_explicit(obj, desired, order)                                            \
	((sizeof *(obj) == 1)	 ? _InterlockedExchange8((CHAR volatile *)obj, (CHAR)desired) :      \
		(sizeof *(obj) == 2) ? _InterlockedExchange16((SHORT volatile *)obj, (SHORT)desired) :   \
		(sizeof *(obj) == 4) ? _InterlockedExchange((LONG volatile *)obj, (LONG)desired) :       \
		(sizeof *(obj) == 8) ? _InterlockedExchange64((LONG64 volatile *)obj, (LONG64)desired) : \
								 (abort(), 0))

inline bool atomic_compare_exchange8(CHAR volatile *obj, CHAR *expected, CHAR desired)
{
	CHAR previous = _InterlockedCompareExchange8(obj, desired, *expected);
	bool result = (previous == *expected);

	if (!result)
	{
		*expected = previous;
	}

	return result;
}

inline bool atomic_compare_exchange16(SHORT volatile *obj, SHORT *expected, SHORT desired)
{
	SHORT previous = _InterlockedCompareExchange16(obj, desired, *expected);
	bool result = (previous == *expected);

	if (!result)
	{
		*expected = previous;
	}

	return result;
}

inline bool atomic_compare_exchange32(LONG volatile *obj, LONG *expected, LONG desired)
{
	LONG previous = _InterlockedCompareExchange(obj, desired, *expected);
	bool result = (previous == *expected);

	if (!result)
	{
		*expected = previous;
	}

	return result;
}

inline bool atomic_compare_exchange64(LONG64 volatile *obj, LONG64 *expected, LONG64 desired)
{
	LONG64 previous = _InterlockedCompareExchange64(obj, desired, *expected);
	bool result = (previous == *expected);

	if (!result)
	{
		*expected = previous;
	}

	return result;
}

#define atomic_compare_exchange_strong_explicit(obj, expected, desired, succ, fail)                                           \
	((sizeof *(obj) == 1)	 ? atomic_compare_exchange8((CHAR volatile *)(obj), (CHAR *)(expected), (CHAR)(desired)) :        \
		(sizeof *(obj) == 2) ? atomic_compare_exchange16((SHORT volatile *)(obj), (SHORT *)(expected), (SHORT)(desired)) :    \
		(sizeof *(obj) == 4) ? atomic_compare_exchange32((LONG volatile *)(obj), (LONG *)(expected), (LONG)(desired)) :       \
		(sizeof *(obj) == 8) ? atomic_compare_exchange64((LONG64 volatile *)(obj), (LONG64 *)(expected), (LONG64)(desired)) : \
								 (abort(), false))

#define atomic_compare_exchange_strong(obj, expected, desired) \
	atomic_compare_exchange_strong_explicit(obj, expected, desired, memory_order_seq_cst, memory_order_seq_cst)

#define atomic_compare_exchange_weak atomic_compare_exchange_strong

#define atomic_compare_exchange_weak_explicit(obj, expected, desired, succ, fail) \
	atomic_compare_exchange_weak((obj), (expected), (desired))

#define atomic_fetch_add(obj, arg) \
	atomic_fetch_add_explicit(obj, arg, memory_order_seq_cst)

#define atomic_fetch_add_explicit(obj, arg, order)                                                \
	((sizeof *(obj) == 1)	 ? _InterlockedExchangeAdd8((char volatile *)obj, (char)(arg)) :      \
		(sizeof *(obj) == 2) ? _InterlockedExchangeAdd16((SHORT volatile *)obj, (SHORT)(arg)) :   \
		(sizeof *(obj) == 4) ? _InterlockedExchangeAdd((LONG volatile *)obj, (LONG)(arg)) :       \
		(sizeof *(obj) == 8) ? _InterlockedExchangeAdd64((LONG64 volatile *)obj, (LONG64)(arg)) : \
								 (abort(), 0))

#define atomic_fetch_sub(obj, arg) \
	atomic_fetch_sub_explicit(obj, arg, memory_order_seq_cst)

#define atomic_fetch_sub_explicit(obj, arg, order) \
	atomic_fetch_add_explicit(obj, (0 - arg), order)

#define atomic_fetch_or(obj, arg) \
	atomic_fetch_or_explicit(obj, arg, memory_order_seq_cst)

#define atomic_fetch_or_explicit(obj, arg, order)                                        \
	((sizeof *(obj) == 1)	 ? _InterlockedOr8((char volatile *)obj, (char)(arg)) :      \
		(sizeof *(obj) == 2) ? _InterlockedOr16((SHORT volatile *)obj, (SHORT)(arg)) :   \
		(sizeof *(obj) == 4) ? _InterlockedOr((LONG volatile *)obj, (LONG)(arg)) :       \
		(sizeof *(obj) == 8) ? _InterlockedOr64((LONG64 volatile *)obj, (LONG64)(arg)) : \
								 (abort(), 0))

#define atomic_fetch_xor(obj, arg) \
	atomic_fetch_xor_explicit(obj, arg, memory_order_seq_cst)

#define atomic_fetch_xor_explicit(obj, arg, order)                                        \
	((sizeof *(obj) == 1)	 ? _InterlockedXor8((char volatile *)obj, (char)(arg)) :      \
		(sizeof *(obj) == 2) ? _InterlockedXor16((SHORT volatile *)obj, (SHORT)(arg)) :   \
		(sizeof *(obj) == 4) ? _InterlockedXor((LONG volatile *)obj, (LONG)(arg)) :       \
		(sizeof *(obj) == 8) ? _InterlockedXor64((LONG64 volatile *)obj, (LONG64)(arg)) : \
								 (abort(), 0))

#define atomic_fetch_and(obj, arg) \
	atomic_fetch_and_explicit(obj, arg, memory_order_seq_cst)

#define atomic_fetch_and_explicit(obj, arg, order)                                        \
	((sizeof *(obj) == 1)	 ? _InterlockedAnd8((char volatile *)obj, (char)(arg)) :      \
		(sizeof *(obj) == 2) ? _InterlockedAnd16((SHORT volatile *)obj, (SHORT)(arg)) :   \
		(sizeof *(obj) == 4) ? _InterlockedAnd((LONG volatile *)obj, (LONG)(arg)) :       \
		(sizeof *(obj) == 8) ? _InterlockedAnd64((LONG64 volatile *)obj, (LONG64)(arg)) : \
								 (abort(), 0))

#define __atomic_compiler_barrier(order)  \
	do                                    \
	{                                     \
		if (order > memory_order_consume) \
		{                                 \
			_ReadWriteBarrier();          \
		}                                 \
	} while (0)

inline void atomic_thread_fence(memory_order order)
{
	__atomic_compiler_barrier(order);

	if (order == memory_order_seq_cst)
	{
		MemoryBarrier();
		__atomic_compiler_barrier(order);
	}
}

inline void atomic_signal_fence(memory_order order)
{
	__atomic_compiler_barrier(order);
}

#undef __atomic_compiler_barrier

#ifdef __cplusplus
}
#endif

#endif /* THREADING_ATOMIC_WIN32_H */
