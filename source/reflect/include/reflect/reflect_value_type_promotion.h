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

#ifndef REFLECT_VALUE_TYPE_PROMOTION_H
#define REFLECT_VALUE_TYPE_PROMOTION_H 1

/* -- Headers -- */

#include <reflect/reflect_api.h>

#include <reflect/reflect_type_id.h>
#include <reflect/reflect_value.h>

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
