/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#ifndef REFLECT_FUNCTION_H
#define REFLECT_FUNCTION_H 1

#include <reflect/reflect_signature.h>
#include <reflect/reflect_value_type.h>

#ifdef __cplusplus
extern "C" {
#endif

struct function_type;

typedef void * function_impl;

typedef struct function_type * function;

typedef void * function_args[];

typedef value function_return;

typedef int (*function_impl_interface_create)(function, function_impl);

typedef function_return (*function_impl_interface_invoke)(function, function_impl, function_args);

typedef void (*function_impl_interface_destroy)(function, function_impl);

typedef struct function_interface_type
{
	function_impl_interface_create create;
	function_impl_interface_invoke invoke;
	function_impl_interface_destroy destroy;

} * function_interface;

typedef function_interface (*function_impl_interface_singleton)(void);

REFLECT_API function function_create(const char * name, size_t args_count, function_impl impl, function_impl_interface_singleton singleton);

REFLECT_API const char * function_name(function func);

REFLECT_API signature function_signature(function func);

REFLECT_API value function_metadata(function func);

REFLECT_API function_return function_call(function func, function_args args);

REFLECT_API void function_destroy(function func);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_FUNCTION_H */
