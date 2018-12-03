/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
*
*/

#ifndef REFLECT_VALUE_TYPE_PROMOTION_H
#define REFLECT_VALUE_TYPE_PROMOTION_H 1

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
*    Promotes a integer value @v to new type @id
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] id
*    New type id of value to be promoted
*
*  @return
*    Promoted value if a valid promotion, or @v if value is already promoted, null otherwhise
*/
REFLECT_API value value_type_promotion_integer(value v, type_id id);

/**
*  @brief
*    Promotes a decimal value @v to new type @id
*
*  @param[in] v
*    Reference to the value
*
*  @param[in] id
*    New type id of value to be promoted
*
*  @return
*    Promoted value if a valid promotion, or @v if value is already promoted, null otherwhise
*/
REFLECT_API value value_type_promotion_decimal(value v, type_id id);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_VALUE_TYPE_PROMOTION_H */
