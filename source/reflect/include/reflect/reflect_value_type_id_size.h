/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef REFLECT_VALUE_TYPE_ID_SIZE_H
#define REFLECT_VALUE_TYPE_ID_SIZE_H 1

/* -- Headers -- */

#include <reflect/reflect_api.h>

#include <reflect/reflect_type_id.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Methods -- */

/**
*  @brief
*    Return size of a type by @id
*
*  @param[in] id
*    Type id used to obtain the size
*
*  @return
*    Size of type assigned to @id
*/
REFLECT_API size_t value_type_id_size(type_id id);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_VALUE_TYPE_ID_SIZE_H */
