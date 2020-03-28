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

#ifndef REFLECT_TYPE_ID_H
#define REFLECT_TYPE_ID_H 1

#include <reflect/reflect_api.h>

#ifdef __cplusplus
extern "C" {
#endif

enum type_primitive_id
{
	TYPE_BOOL		= 0,
	TYPE_CHAR		= 1,
	TYPE_SHORT		= 2,
	TYPE_INT		= 3,
	TYPE_LONG		= 4,
	TYPE_FLOAT		= 5,
	TYPE_DOUBLE		= 6,
	TYPE_STRING		= 7,
	TYPE_BUFFER		= 8,
	TYPE_ARRAY		= 9,
	TYPE_MAP		= 10,
	TYPE_PTR		= 11,
	TYPE_FUTURE		= 12,
	TYPE_FUNCTION	= 13,
	TYPE_NULL		= 14,

	TYPE_SIZE,
	TYPE_INVALID
};

typedef int type_id;

/**
*  @brief
*    Obtain a human readable form of the type id @id
*
*  @param[in] id
*    Type id to be converted into string
*
*  @return
*    String representing a readable name for @id
*/
REFLECT_API const char * type_id_name(type_id id);

/**
*  @brief
*    Check if type id is boolean value (bool)
*
*  @param[in] id
*    Type id to be checked
*
*  @return
*    Returns zero if type is integer, different from zero otherwhise
*/
REFLECT_API int type_id_boolean(type_id id);

/**
*  @brief
*    Check if type id is char value (char)
*
*  @param[in] id
*    Type id to be checked
*
*  @return
*    Returns zero if type is char, different from zero otherwhise
*/
REFLECT_API int type_id_char(type_id id);

/**
*  @brief
*    Check if type id is integer value (bool, char, short, int, long)
*
*  @param[in] id
*    Type id to be checked
*
*  @return
*    Returns zero if type is integer, different from zero otherwhise
*/
REFLECT_API int type_id_integer(type_id id);

/**
*  @brief
*    Check if type id is decimal value (float, double)
*
*  @param[in] id
*    Type id to be checked
*
*  @return
*    Returns zero if type is decimal, different from zero otherwhise
*/
REFLECT_API int type_id_decimal(type_id id);

/**
*  @brief
*    Check if type id is string value (char *)
*
*  @param[in] id
*    Type id to be checked
*
*  @return
*    Returns zero if type is string, different from zero otherwhise
*/
REFLECT_API int type_id_string(type_id id);

/**
*  @brief
*    Check if type id is buffer value (void *)
*
*  @param[in] id
*    Type id to be checked
*
*  @return
*    Returns zero if type is buffer, different from zero otherwhise
*/
REFLECT_API int type_id_buffer(type_id id);

/**
*  @brief
*    Check if type id is array of values (value *)
*
*  @param[in] id
*    Type id to be checked
*
*  @return
*    Returns zero if type is array, different from zero otherwhise
*/
REFLECT_API int type_id_array(type_id id);

/**
*  @brief
*    Check if type id is map of values (string -> value)
*
*  @param[in] id
*    Type id to be checked
*
*  @return
*    Returns zero if type is map, different from zero otherwhise
*/
REFLECT_API int type_id_map(type_id id);

/**
*  @brief
*    Check if type id is pointer value (void *)
*
*  @param[in] id
*    Type id to be checked
*
*  @return
*    Returns zero if type is pointer, different from zero otherwhise
*/
REFLECT_API int type_id_ptr(type_id id);

/**
*  @brief
*    Check if type id is future value (future)
*
*  @param[in] id
*    Type id to be checked
*
*  @return
*    Returns zero if type is future, different from zero otherwhise
*/
REFLECT_API int type_id_future(type_id id);

/**
*  @brief
*    Check if type id is function value (function)
*
*  @param[in] id
*    Type id to be checked
*
*  @return
*    Returns zero if type is function, different from zero otherwhise
*/
REFLECT_API int type_id_function(type_id id);

/**
*  @brief
*    Check if type id is pointer value NULL
*
*  @param[in] id
*    Type id to be checked
*
*  @return
*    Returns zero if type is null, different from zero otherwhise
*/
REFLECT_API int type_id_null(type_id id);

/**
*  @brief
*    Check if type id is invalid
*
*  @param[in] id
*    Type id to be checked
*
*  @return
*    Returns zero if type is invalid, different from zero otherwhise
*/
REFLECT_API int type_id_invalid(type_id id);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_TYPE_ID_H */
