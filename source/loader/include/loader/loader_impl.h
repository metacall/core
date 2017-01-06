/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#ifndef LOADER_IMPL_H
#define LOADER_IMPL_H 1

#include <loader/loader_api.h>

#include <loader/loader_path.h>
#include <loader/loader_impl_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

LOADER_API loader_impl loader_impl_create(const char * path, loader_naming_extension extension);

LOADER_API loader_impl_data loader_impl_get(loader_impl impl);

LOADER_API loader_impl_interface loader_impl_symbol(loader_impl impl);

LOADER_API loader_naming_extension * loader_impl_extension(loader_impl impl);

LOADER_API context loader_impl_context(loader_impl impl);

LOADER_API type loader_impl_type(loader_impl impl, const char * name);

LOADER_API int loader_impl_type_define(loader_impl impl, const char * name, type t);

LOADER_API int loader_impl_execution_path(loader_impl impl, const loader_naming_path path);

LOADER_API int loader_impl_load_from_file(loader_impl impl, const loader_naming_path path);

LOADER_API int loader_impl_load_from_memory(loader_impl impl, const loader_naming_extension extension, const char * buffer, size_t size);

LOADER_API void loader_impl_destroy(loader_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_IMPL_H */
