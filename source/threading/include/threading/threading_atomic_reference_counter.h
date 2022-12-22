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

#ifndef THREADING_ATOMIC_REFERENCE_COUNTER_H
#define THREADING_ATOMIC_REFERENCE_COUNTER_H 1

/* -- Headers -- */

#include<threading/threading_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#if defined(THREADING_ATOMIC_REFERENCE_COUNTER_H) && THREADING_ATOMIC_REFERENCE_COUNTER_H==1

  #include<threading/threading_atomic.h>

  #define threading_atomic_reference_counter(name) \
    static struct                \
    {                              \
      atomic_uint reference_counter;       \
    } name={0}

  #define threading_atomic_reference_counter_increment(name) \
    atomic_fetch_add_explicit(&name.reference_counter, 1, memory_order_relaxed)

  #define threading_atomic_reference_counter_decrement(name) \
    atomic_fetch_sub_explicit(&name.reference_counter, 1, memory_order_release)

#else
  #define threading_atomic_reference_counter(name) \
    typedef char threading_atomic_reference_counter_disabled

  #define threading_atomic_reference_counter_increment(name) \
    do                                                       \
    {                                                        \
    } while(0) 

  #define threading_atomic_reference_counter_decrement(name) \
    do                                                       \
    {                                                        \
    } while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif






  


