/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
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
	TYPE_BOOL	= 0,
	TYPE_CHAR	= 1,
	TYPE_SHORT	= 2,
	TYPE_INT	= 3,
	TYPE_LONG	= 4,
	TYPE_FLOAT	= 5,
	TYPE_DOUBLE	= 6,
	TYPE_STRING = 7,
	TYPE_BUFFER	= 8,
	TYPE_ARRAY	= 9,
	TYPE_MAP	= 10,
	TYPE_PTR	= 11,
	TYPE_NULL	= 12,

	TYPE_SIZE,
	TYPE_INVALID
};

typedef int type_id;

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
