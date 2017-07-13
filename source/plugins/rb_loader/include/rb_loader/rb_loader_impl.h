/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading ruby code at run-time into a process.
 *
 */

#ifndef RB_LOADER_IMPL_H
#define RB_LOADER_IMPL_H 1

#include <rb_loader/rb_loader_api.h>

#include <loader/loader_impl_interface.h>

#include <configuration/configuration.h>

#ifdef __cplusplus
extern "C" {
#endif

RB_LOADER_API loader_impl_data rb_loader_impl_initialize(loader_impl impl, configuration config, loader_host host);

RB_LOADER_API int rb_loader_impl_execution_path(loader_impl impl, const loader_naming_path path);

RB_LOADER_API loader_handle rb_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size);

RB_LOADER_API loader_handle rb_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size);

RB_LOADER_API loader_handle rb_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path);

RB_LOADER_API int rb_loader_impl_clear(loader_impl impl, loader_handle handle);

RB_LOADER_API int rb_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx);

RB_LOADER_API int rb_loader_impl_destroy(loader_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* RB_LOADER_IMPL_H */
