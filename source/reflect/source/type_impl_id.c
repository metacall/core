/************************************************************************/
/*	Reflect Library by Parra Studios									*/
/*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																		*/
/*	A library for provide reflection and metadata representation.		*/
/*																		*/
/************************************************************************/

#include <reflect/type_impl_id.h>

int type_impl_id_primitive(type_impl_id id)
{
	return !(id < TYPE_IMPL_ID_INVALID);
}

int type_impl_id_runtime(type_impl_id id)
{
	return !(id > TYPE_IMPL_ID_INVALID);
}

int type_impl_id_invalid(type_impl_id id)
{
	return !(id == TYPE_IMPL_ID_INVALID);
}
