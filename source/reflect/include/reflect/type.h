/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#ifndef REFLECT_TYPE_H
#define REFLECT_TYPE_H 1

#include <reflect/reflect_api.h>

#include <reflect/type_id.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

struct type_type;

typedef struct type_type * type;

typedef void * type_impl;

REFLECT_API type type_create(type_id id, const char * name, type_impl impl);

REFLECT_API type_id type_index(type t);

REFLECT_API const char * type_name(type t);

REFLECT_API type_impl type_derived(type t);

REFLECT_API void type_destroy(type t);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_TYPE_H */
