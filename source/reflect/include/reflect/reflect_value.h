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

#ifndef REFLECT_VALUE_H
#define REFLECT_VALUE_H 1

/* -- Headers -- */

#include <reflect/reflect_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Definitions -- */

#if !defined(boolean)
	#define boolean unsigned char
#endif

/* -- Type Definitions -- */

typedef void *value;

typedef void (*value_finalizer_cb)(value, void *);

/* -- Methods -- */

/**
*  @brief
*    Reserve memory for a value with size @bytes
*
*  @param[in] bytes
*    Size in bytes to be allocated
*
*  @return
*    Pointer to uninitialized value if success, null otherwhise
*/
REFLECT_API value value_alloc(size_t bytes);

/**
*  @brief
*    Create a value from @data with size @bytes
*
*  @param[in] data
*    Pointer to memory block
*
*  @param[in] bytes
*    Size in bytes of the memory block @data
*
*  @return
*    Pointer to value if success, null otherwhise
*/
REFLECT_API value value_create(const void *data, size_t bytes);

/**
*  @brief
*    Check if the value @v is valid or the memory is corrupted
*
*  @param[in] v
*    Reference of value to be checked
*
*  @return
*    Zero if the value is valid, null otherwhise
*/
REFLECT_API int value_validate(value v);

/**
*  @brief
*    Copy a value from @v
*
*  @param[in] v
*    Reference of value to be copied
*
*  @return
*    Reference to new value copied from @v
*/
REFLECT_API value value_copy(value v);

/**
*  @brief
*    Copies the ownership from @src to @dst, including the finalizer,
*    and resets the owner and finalizer of @src
*
*  @param[in] src
*    Source value which will lose the ownership
*
*  @param[in] dst
*    Destination value which will recieve the ownership
*/
REFLECT_API void value_move(value src, value dst);

/**
*  @brief
*    Returns the size of the value
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Size in bytes of the value
*/
REFLECT_API size_t value_size(value v);

/**
*  @brief
*    Increment reference count of a value
*
*  @param[in] v
*    Reference to the value
*/
REFLECT_API void value_ref_inc(value v);

/**
*  @brief
*    Decrement reference count of a value
*
*  @param[in] v
*    Reference to the value
*/
REFLECT_API void value_ref_dec(value v);

/**
*  @brief
*    Set up the value finalizer, a callback that
*    will be executed when the value life ends
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] finalizer
*    Reference to the callback
*
*  @param[in] finalizer_data
*    Reference to additional data to be passed when the finalizer is called
*/
REFLECT_API void value_finalizer(value v, value_finalizer_cb finalizer, void *finalizer_data);

/**
*  @brief
*    Get pointer reference to value data
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Pointer to writeable memory block of value @v
*/
REFLECT_API void *value_data(value v);

/**
*  @brief
*    Convert value @v to memory block @data
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] data
*    Pointer to memory block where value will be stored
*
*  @param[in] bytes
*    Size in bytes of the memory block @data
*/
REFLECT_API void value_to(value v, void *data, size_t bytes);

/**
*  @brief
*    Assing memory block @data to value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] data
*    Pointer to memory block to be copied into @v
*
*  @param[in] bytes
*    Size in bytes of the memory block @data
*
*  @return
*    Value with @data of size @bytes assigned to it
*/
REFLECT_API value value_from(value v, const void *data, size_t bytes);

/**
*  @brief
*    Destroy a value from scope stack
*
*  @param[in] v
*    Reference to the value
*/
REFLECT_API void value_destroy(value v);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_VALUE_H */
