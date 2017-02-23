/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
*
*/

#ifndef REFLECT_VALUE_TYPE_STRINGIFY_H
#define REFLECT_VALUE_TYPE_STRINGIFY_H 1

/* -- Headers -- */

#include <reflect/reflect_api.h>

#include <reflect/reflect_value.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Convert to string the value @v
*
*  @param[in] v
*    Reference to the value
*
*  @param[out] dest
*    Destination address where string will be stored
*
*  @param[in] size
*    Size of buffer @dest in bytes
*
*  @param[out] length
*    Length of string stored in @dest
*/
REFLECT_API void value_stringify(value v, char * dest, size_t size, size_t * length);

/**
*  @brief
*    Convert to string value the value @v
*
*  @param[in] v
*    Reference to the value
*
*  @return
*    Reference to the new string value (and destroy
*    old value @v from memory) if success, null (and
*    destroy is not performed) otherwise
*/
REFLECT_API value value_type_stringify(value v);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_VALUE_TYPE_STRINGIFY_H */
