/************************************************************************/
/*	Reflect Library by Parra Studios									*/
/*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																		*/
/*	A library for provide reflection and metadata representation.		*/
/*																		*/
/************************************************************************/

#ifndef REFLECT_TYPE_PRIMITIVES_H
#define REFLECT_TYPE_PRIMITIVES_H 1

#include <reflect/reflect_api.h>

#ifdef __cplusplus
extern "C" {
#endif

enum type_primitive_id
{
	TYPE_CHAR	= 0x00,
	TYPE_UCHAR	= 0x01,
	TYPE_SHORT	= 0x02,
	TYPE_USHORT	= 0x03,
	TYPE_INT	= 0x04,
	TYPE_UINT	= 0x05,
	TYPE_LONG	= 0x06,
	TYPE_ULONG	= 0x07,
	TYPE_FLOAT	= 0x08,
	TYPE_DOUBLE	= 0x09,
	TYPE_PTR	= 0x0A,

	TYPE_COUNT
};

struct type_type;

typedef struct type_type * type;

typedef void * type_impl;

typedef type_impl (*type_primitive_impl)(enum type_primitive_id);

REFLECT_API type type_primitive(enum type_primitive_id primitive_id);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_TYPE_PRIMITIVES_H */
