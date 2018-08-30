/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading python code at run-time into a process.
 *
 */

#ifndef PY_LOADER_IMPL_H
#define PY_LOADER_IMPL_H 1

#include <py_loader/py_loader_api.h>

#include <loader/loader_impl_interface.h>

#include <configuration/configuration.h>

#ifdef __cplusplus
extern "C" {
#endif

PY_LOADER_API loader_impl_data py_loader_impl_initialize(loader_impl impl, configuration config, loader_host host);

PY_LOADER_API int py_loader_impl_execution_path(loader_impl impl, const loader_naming_path path);

PY_LOADER_API loader_handle py_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size);

PY_LOADER_API loader_handle py_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size);

PY_LOADER_API loader_handle py_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path);

PY_LOADER_API int py_loader_impl_clear(loader_impl impl, loader_handle handle);

PY_LOADER_API int py_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx);

PY_LOADER_API int py_loader_impl_destroy(loader_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* PY_LOADER_IMPL_H */
