/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#ifndef REFLECT_TYPE_IMPL_ID_H
#define REFLECT_TYPE_IMPL_ID_H 1

#include <reflect/reflect_api.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TYPE_IMPL_ID_INVALID 0x00

typedef int type_impl_id;

REFLECT_API int type_impl_id_primitive(type_impl_id id);

REFLECT_API int type_impl_id_runtime(type_impl_id id);

REFLECT_API int type_impl_id_invalid(type_impl_id id);

#endif /* REFLECT_TYPE_IMPL_ID_H */
