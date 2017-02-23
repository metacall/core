/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
*
*/

#ifndef REFLECT_VALUE_TYPE_PARSE_H
#define REFLECT_VALUE_TYPE_PARSE_H 1

/* -- Headers -- */

#include <reflect/reflect_api.h>

#include <reflect/reflect_value.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Convert to string value @v to a parsed value
*
*  @param[in] v
*    Reference to the value to be parsed
*
*  @param[in] id
*    Result type will have the returned value when parsed
*
*  @return
*    Reference to the new parsed value (and destroy
*    old string value @v from memory) if success, null (and
*    destroy is not performed) otherwise
*/
REFLECT_API value value_type_parse(value v, type_id id);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_VALUE_TYPE_PARSE_H */
