/************************************************************************/
/*	Reflect Library by Parra Studios									*/
/*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																		*/
/*	A library for provide reflection and metadata representation.		*/
/*																		*/
/************************************************************************/

#ifndef REFLECT_TYPE_H
#define REFLECT_TYPE_H 1

#include <reflect/reflect_api.h>

#include <reflect/type_primitives.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

struct type_type;

typedef struct type_type * type;

typedef void * type_impl;

typedef void * type_instance;

typedef type_impl (*type_create_impl)(type);

typedef type_instance (*type_instance_create_impl)(type);

typedef type_instance (*type_instance_create_array_impl)(type, int);

typedef void (*type_instance_destroy_impl)(type, type_instance);

typedef void (*type_destroy_impl)(type, type_impl);

typedef struct type_interface_type
{
	type_create_impl create;
	type_instance_create_impl instance_create;
	type_instance_create_array_impl instance_create_array;
	type_instance_destroy_impl instance_destroy_array;
	type_destroy_impl destroy;

} * type_interface;

REFLECT_API type type_create(int id, char * name, size_t size, type_interface interface);

REFLECT_API int type_id(type t);

REFLECT_API char * type_name(type t);

REFLECT_API size_t type_size(type t);

REFLECT_API void type_destroy(type t);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_TYPE_H */
