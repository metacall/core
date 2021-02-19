/*
 *	Thrading Library by Parra Studios
 *	A threading library providing utilities for lock-free data structures and more.
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

#ifndef THREADING_THREAD_ID_H
#define THREADING_THREAD_ID_H 1

/* -- Headers -- */

#include <threading/threading_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct thread_os_id_type;

/* -- Type Definitions -- */

typedef struct thread_os_id_type * thread_os_id;

/* -- Macros -- */

/**
*  @brief
*    Return a new pointer to the id of the current thread (must be freed)
*
*  @return
*    Returns pointer to thread id implementation on correct creation, null otherwise
*/
THREADING_API thread_os_id thread_id_get_current(void);

/**
*  @brief
*    Compare if two thread ids are equal
*
*  @param[in] left
*    The left operand of the thread id comparison
*
*  @param[in] right
*    The right operand of the thread id comparison
*
*  @return
*    Returns 0 if they are equal, 1 if they are different
*/
THREADING_API int thread_id_compare(thread_os_id left, thread_os_id right);

/**
*  @brief
*    Cleans up memory associated to the thread id
*
*  @param[in] id
*    The thread id pointer to be destroyed
*/
THREADING_API void thread_id_destroy(thread_os_id id);


#ifdef __cplusplus
}
#endif

#endif /* THREADING_THREAD_ID_H */
