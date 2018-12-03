/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading c code at run-time into a process.
 *
 */

#ifndef C_LOADER_IMPL_H
#define C_LOADER_IMPL_H 1

#include <c_loader/c_loader_api.h>

#include <loader/loader_impl_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

C_LOADER_API loader_impl_data c_loader_impl_initialize(loader_impl impl, configuration config, loader_host host);

C_LOADER_API int c_loader_impl_execution_path(loader_impl impl, const loader_naming_path path);

C_LOADER_API loader_handle c_loader_impl_load(loader_impl impl, const loader_naming_path path, loader_naming_name name);

C_LOADER_API int c_loader_impl_clear(loader_impl impl, loader_handle handle);

C_LOADER_API int c_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx);

C_LOADER_API int c_loader_impl_destroy(loader_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* C_LOADER_IMPL_H */
