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

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>

#ifdef __cplusplus
extern "C" {
#endif

struct scope_type;

typedef void * scope_object;

typedef size_t scope_stack_ptr;

typedef struct scope_type * scope;

REFLECT_API scope scope_create(const char * name);

REFLECT_API size_t scope_size(scope sp);

REFLECT_API int scope_define(scope sp, const char * key, scope_object obj);

REFLECT_API void scope_print(scope sp);

REFLECT_API scope_object scope_get(scope sp, const char * key);

REFLECT_API scope_object scope_undef(scope sp, const char * key);

REFLECT_API int scope_append(scope dest, scope src);

REFLECT_API size_t * scope_stack_return(scope sp);

REFLECT_API scope_stack_ptr scope_stack_push(scope sp, size_t bytes);

REFLECT_API void * scope_stack_get(scope sp, scope_stack_ptr stack_ptr);

REFLECT_API int scope_stack_pop(scope sp);

REFLECT_API void scope_destroy(scope sp);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_SCOPE_H */
