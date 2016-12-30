/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
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
