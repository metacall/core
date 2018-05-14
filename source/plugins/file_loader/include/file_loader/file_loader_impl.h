/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading file code at run-time into a process.
 *
 */

#ifndef FILE_LOADER_IMPL_H
#define FILE_LOADER_IMPL_H 1

#include <file_loader/file_loader_api.h>

#include <loader/loader_impl_interface.h>

#include <configuration/configuration.h>

#ifdef __cplusplus
extern "C" {
#endif

FILE_LOADER_API loader_impl_data file_loader_impl_initialize(loader_impl impl, configuration config, loader_host host);

FILE_LOADER_API int file_loader_impl_execution_path(loader_impl impl, const loader_naming_path path);

FILE_LOADER_API loader_handle file_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size);

FILE_LOADER_API loader_handle file_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size);

FILE_LOADER_API loader_handle file_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path);

FILE_LOADER_API int file_loader_impl_clear(loader_impl impl, loader_handle handle);

FILE_LOADER_API int file_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx);

FILE_LOADER_API int file_loader_impl_destroy(loader_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* FILE_LOADER_IMPL_H */
