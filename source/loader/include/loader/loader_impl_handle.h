/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#ifndef LOADER_IMPL_HANDLE_H
#define LOADER_IMPL_HANDLE_H 1

#include <loader/loader_impl.h>
#include <loader/loader_handle.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef loader_handle (*loader_impl_handle_create)(loader_impl, const loader_naming_path, const loader_naming_name);

typedef int (*loader_impl_handle_discover)(loader_impl, loader_handle, context);

typedef int (*loader_impl_handle_destroy)(loader_impl, loader_handle);

typedef struct loader_impl_handle_type
{
	loader_impl_handle_create create;
	loader_impl_handle_discover discover;
	loader_impl_handle_destroy destroy;

} * loader_impl_handle;

typedef loader_impl_handle (*loader_impl_handle_singleton)(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_IMPL_HANDLE_H */
