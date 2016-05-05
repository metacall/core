/************************************************************************/
/*	Reflect Library by Parra Studios									*/
/*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																		*/
/*	A library for provide reflection and metadata representation.		*/
/*																		*/
/************************************************************************/

#include <reflect/type_impl.h>
#include <reflect/type_primitives.h>

type type_primitive(enum type_primitive_id primitive_id)
{
	if (primitive_id != TYPE_COUNT)
	{
		static struct type_type primitive_types[TYPE_COUNT] =
		{
			{ -1,	"char",		sizeof(char) },				/* CHAR */
			{ -2,	"uchar",	sizeof(unsigned char) },	/* UCHAR */
			{ -3,	"short",	sizeof(short) },			/* SHORT */
			{ -4,	"ushort",	sizeof(unsigned short) },	/* USHORT */
			{ -5,	"int",		sizeof(int) },				/* INT */
			{ -6,	"uint",		sizeof(unsigned int) },		/* UINT */
			{ -7,	"long",		sizeof(long) },				/* LONG */
			{ -8,	"ulong",	sizeof(unsigned long) },	/* ULONG */
			{ -9,	"float",	sizeof(float) },			/* FLOAT */
			{ -10,	"double",	sizeof(double) },			/* DOUBLE */
			{ -11,	"ptr",		sizeof(void *) }			/* PTR */

		};

		return &primitive_types[primitive_id];
	}

	return NULL;
}
