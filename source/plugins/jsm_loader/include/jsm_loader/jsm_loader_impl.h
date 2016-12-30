/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading javascript code at run-time into a process.
 *
 */

#ifndef JSM_LOADER_IMPL_H
#define JSM_LOADER_IMPL_H 1

#include <jsm_loader/jsm_loader_api.h>

#include <loader/loader_impl_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

JSM_LOADER_API loader_impl_data jsm_loader_impl_initialize(loader_impl impl);

JSM_LOADER_API int jsm_loader_impl_execution_path(loader_impl impl, const loader_naming_path path);

JSM_LOADER_API loader_handle jsm_loader_impl_load(loader_impl impl, const loader_naming_path path, loader_naming_name name);

JSM_LOADER_API int jsm_loader_impl_clear(loader_impl impl, loader_handle handle);

JSM_LOADER_API int jsm_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx);

JSM_LOADER_API int jsm_loader_impl_destroy(loader_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* JSM_LOADER_IMPL_H */
