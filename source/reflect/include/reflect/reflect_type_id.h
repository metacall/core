/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#define TYPE_INVALID ((type_id)0x00)

enum type_primitive_id
{
	TYPE_BOOL	= -1,
	TYPE_CHAR	= -2,
	TYPE_INT	= -3,
	TYPE_LONG	= -4,
	TYPE_DOUBLE = -5,
	TYPE_STRING = -6,
	TYPE_PTR	= -7
};

typedef int type_id;

REFLECT_API int type_id_primitive(type_id id);

REFLECT_API int type_id_runtime(type_id id);

REFLECT_API int type_id_invalid(type_id id);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_TYPE_ID_H */
