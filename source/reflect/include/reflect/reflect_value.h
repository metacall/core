/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
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
#	define boolean unsigned char
#endif

/* -- Type Definitions -- */

typedef void * value;

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
REFLECT_API value value_create(const void * data, size_t bytes);

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
*    Get pointer reference to value data
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Pointer to writeable memory block of value @v
*/
REFLECT_API void * value_data(value v);

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
REFLECT_API void value_to(value v, void * data, size_t bytes);

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
REFLECT_API value value_from(value v, const void * data, size_t bytes);

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
