/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#ifndef REFLECT_TYPE_IMPL_H
#define REFLECT_TYPE_IMPL_H 1

#include <reflect/reflect_api.h>

#include <reflect/type_impl_id.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

struct type_interface_type;

typedef void * type_impl;

typedef void * type_instance;

typedef struct type_interface_type * type_interface;

typedef struct type_type
{
	type_impl_id id;
	char * name;
	size_t size;
	type_interface interface;
	type_impl impl;
} * type;

typedef type_interface (*type_interface_impl)(void);

REFLECT_API type_impl type_create_impl_null(type t);

REFLECT_API type_instance type_instance_create_impl_null(type t);

REFLECT_API type_instance type_instance_create_array_impl_null(type t, int count);

REFLECT_API void type_instance_destroy_impl_null(type t, type_instance instance);

REFLECT_API void type_destroy_impl_null(type t, type_impl impl);

REFLECT_API type_interface type_interface_impl_null(void);

#endif /* REFLECT_TYPE_IMPL_H */
