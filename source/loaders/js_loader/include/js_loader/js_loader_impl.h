/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading javascript code at run-time into a process.
 *
 */

#ifndef JS_LOADER_IMPL_H
#define JS_LOADER_IMPL_H 1

#include <js_loader/js_loader_api.h>

#include <loader/loader_impl_interface.h>
#include <configuration/configuration.h>

#ifdef __cplusplus
extern "C" {
#endif

JS_LOADER_API loader_impl_data js_loader_impl_initialize(loader_impl impl, configuration config, loader_host host);

JS_LOADER_API int js_loader_impl_execution_path(loader_impl impl, const loader_naming_path path);

JS_LOADER_API loader_handle js_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size);

JS_LOADER_API loader_handle js_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size);

JS_LOADER_API loader_handle js_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path);

JS_LOADER_API int js_loader_impl_clear(loader_impl impl, loader_handle handle);

JS_LOADER_API int js_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx);

JS_LOADER_API int js_loader_impl_destroy(loader_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* JS_LOADER_IMPL_H */
