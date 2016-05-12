/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#ifndef REFLECT_SCOPE_H
#define REFLECT_SCOPE_H 1

#include <reflect/reflect_api.h>

#include <reflect/type.h>
#include <reflect/function.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void * scope_object;

typedef struct scope_type * scope;

REFLECT_API scope scope_create(char * name);

REFLECT_API int scope_define(scope s, char * key, scope_object obj);

REFLECT_API scope_object scope_get(scope s, char * key);

REFLECT_API scope_object scope_undef(scope s, char * key);

REFLECT_API void scope_destroy(scope s);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_SCOPE_H */
