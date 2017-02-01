/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading mock code at run-time into a process.
 *
 */

#ifndef MOCK_LOADER_IMPL_H
#define MOCK_LOADER_IMPL_H 1

#include <mock_loader/mock_loader_api.h>

#include <loader/loader_impl_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

MOCK_LOADER_API loader_impl_data mock_loader_impl_initialize(loader_impl impl);

MOCK_LOADER_API int mock_loader_impl_execution_path(loader_impl impl, const loader_naming_path path);

MOCK_LOADER_API loader_handle mock_loader_impl_load_from_files(loader_impl impl, const loader_naming_name name, const loader_naming_path path[], size_t size);

MOCK_LOADER_API loader_handle mock_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const loader_naming_extension extension, const char * buffer, size_t size);

MOCK_LOADER_API int mock_loader_impl_clear(loader_impl impl, loader_handle handle);

MOCK_LOADER_API int mock_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx);

MOCK_LOADER_API int mock_loader_impl_destroy(loader_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_LOADER_IMPL_H */
