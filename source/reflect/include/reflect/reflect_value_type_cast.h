/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
*
*/

#ifndef REFLECT_VALUE_TYPE_CAST_H
#define REFLECT_VALUE_TYPE_CAST_H 1

/* -- Headers -- */

#include <reflect/reflect_api.h>

#include <reflect/reflect_value.h>
#include <reflect/reflect_type_id.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Casts a value to a new type @id
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] id
*    New type id of value to be casted
*
*  @return
*    Casted value or reference to @v if casting is between equivalent types
*/
REFLECT_API value value_type_cast(value v, type_id id);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_VALUE_TYPE_CAST_H */
