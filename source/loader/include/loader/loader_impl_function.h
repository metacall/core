/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#ifndef LOADER_IMPL_FUNCTION_H
#define LOADER_IMPL_FUNCTION_H 1

#include <loader/loader_function.h>
#include <loader/loader_handle.h>
#include <loader/loader_impl.h>

#include <reflect/function.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef loader_function (*loader_impl_function_create)(loader_impl, loader_handle);

typedef function_impl_interface_singleton (*loader_impl_function_interface)(void);

typedef int (*loader_impl_function_destroy)(loader_impl, loader_handle, loader_function);

typedef struct loader_impl_function_type
{
	loader_impl_function_create create;
	loader_impl_function_interface interface;
	loader_impl_function_destroy destroy;

} * loader_impl_function;

typedef loader_impl_function (*loader_impl_function_singleton)(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_IMPL_FUNCTION_H */
