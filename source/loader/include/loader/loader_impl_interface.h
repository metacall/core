/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#ifndef LOADER_IMPL_INTERFACE_H
#define LOADER_IMPL_INTERFACE_H 1

#include <loader/loader_api.h>

#include <loader/loader_handle.h>
#include <loader/loader_naming_type.h>

#ifdef __cplusplus
extern "C" {
#endif

struct loader_impl_type;

typedef struct loader_impl_type * loader_impl;

struct context_type;

typedef struct context_type * context;

typedef int (*loader_impl_interface_initialize)(loader_impl);

typedef int (*loader_impl_interface_execution_path)(loader_impl, loader_naming_path);

typedef loader_handle (*loader_impl_interface_load)(loader_impl, loader_naming_name);

typedef int (*loader_impl_interface_clear)(loader_impl, loader_handle);

typedef int (*loader_impl_interface_discover)(loader_impl, loader_handle, context);

typedef int (*loader_impl_interface_destroy)(loader_impl);

typedef struct loader_impl_interface_type
{
	loader_impl_interface_initialize initialize;
	loader_impl_interface_execution_path execution_path;
	loader_impl_interface_load load;
	loader_impl_interface_clear clear;
	loader_impl_interface_discover discover;
	loader_impl_interface_destroy destroy;

} * loader_impl_interface;

typedef loader_impl_interface (*loader_impl_interface_singleton)(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_IMPL_INTERFACE_H */
