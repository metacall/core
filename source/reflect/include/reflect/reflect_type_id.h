/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
	TYPE_PTR	= 8,

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
*    Check if type id is pointer (void *)
*
*  @param[in] id
*    Type id to be checked
*
*  @return
*    Returns zero if type is pointer, different from zero otherwhise
*/
REFLECT_API int type_id_pointer(type_id id);

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
