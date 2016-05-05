/************************************************************************/
/*	Reflect Library by Parra Studios									*/
/*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																		*/
/*	A library for provide reflection and metadata representation.		*/
/*																		*/
/************************************************************************/

#ifndef REFLECT_SIGNATURE_H
#define REFLECT_SIGNATURE_H 1

#include <reflect/reflect_api.h>

#include <reflect/type.h>

#ifdef __cplusplus
extern "C" {
#endif

struct signature_type;

typedef struct signature_type * signature;

REFLECT_API signature signature_create(int count);

REFLECT_API int signature_count(signature s);

REFLECT_API char * signature_get_name(signature s, int index);

REFLECT_API type signature_get_type(signature s, int index);

REFLECT_API void signature_set(signature s, int index, char * name, type t);

REFLECT_API void signature_destroy(signature s);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_SIGNATURE_H */
