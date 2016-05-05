/************************************************************************/
/*	Reflect Library by Parra Studios									*/
/*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																		*/
/*	A library for provide reflection and metadata representation.		*/
/*																		*/
/************************************************************************/

#ifndef REFLECT_FUNCTION_H
#define REFLECT_FUNCTION_H 1

#include <reflect/signature.h>

#ifdef __cplusplus
extern "C" {
#endif

struct function_type;

typedef void * function_impl;

typedef struct function_type * function;

typedef void ** function_args;

typedef function_impl (*function_create_impl)(function);

typedef void (*function_call_impl)(function, function_impl, function_args);

typedef void (*function_destroy_impl)(function, function_impl);

typedef struct function_interface_type
{
	function_create_impl create;
	function_call_impl invoke;
	function_destroy_impl destroy;

} * function_interface;

REFLECT_API function function_create(char * name, signature s, function_interface interface);

REFLECT_API char * function_name(function func);

REFLECT_API signature function_signature(function func);

REFLECT_API void function_call(function func, function_args args);

REFLECT_API void function_destroy(function func);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_FUNCTION_H */
