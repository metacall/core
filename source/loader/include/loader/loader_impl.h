/*
 *	Loader Library by Parra Studios
 *	A library for loading executable code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

#ifndef LOADER_IMPL_H
#define LOADER_IMPL_H 1

#include <loader/loader_api.h>

#include <loader/loader_impl_interface.h>
#include <loader/loader_naming.h>

#include <plugin/plugin_impl.h>
#include <plugin/plugin_manager.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

LOADER_API int loader_impl_is_initialized(loader_impl impl);

LOADER_API loader_impl loader_impl_create(const loader_tag tag);

LOADER_API loader_impl loader_impl_create_host(const loader_tag tag);

LOADER_API void loader_impl_attach(loader_impl impl, plugin p);

LOADER_API plugin loader_impl_plugin(loader_impl impl);

LOADER_API loader_impl_data loader_impl_get(loader_impl impl);

LOADER_API value loader_impl_get_value(loader_impl impl, const char *name);

LOADER_API context loader_impl_context(loader_impl impl);

LOADER_API type loader_impl_type(loader_impl impl, const char *name);

LOADER_API int loader_impl_type_define(loader_impl impl, const char *name, type t);

LOADER_API int loader_impl_execution_path(plugin p, loader_impl impl, const loader_path path);

LOADER_API int loader_impl_load_from_file(plugin_manager manager, plugin p, loader_impl impl, const loader_path paths[], size_t size, void **handle_ptr);

LOADER_API int loader_impl_load_from_memory(plugin_manager manager, plugin p, loader_impl impl, const char *buffer, size_t size, void **handle_ptr);

LOADER_API int loader_impl_load_from_package(plugin_manager manager, plugin p, loader_impl impl, const loader_path path, void **handle_ptr);

LOADER_API void *loader_impl_get_handle(loader_impl impl, const char *name);

LOADER_API void loader_impl_set_options(loader_impl impl, void *options);

LOADER_API value loader_impl_get_options(loader_impl impl);

LOADER_API value loader_impl_get_option(loader_impl impl, const char *field);

LOADER_API int loader_impl_get_option_host(loader_impl impl);

LOADER_API int loader_impl_handle_initialize(plugin_manager manager, plugin p, loader_impl impl, const loader_path name, void **handle_ptr);

LOADER_API vector loader_impl_handle_populated(void *handle);

LOADER_API const char *loader_impl_handle_id(void *handle);

LOADER_API value loader_impl_handle_export(void *handle);

LOADER_API context loader_impl_handle_context(void *handle);

LOADER_API void *loader_impl_handle_container_of(loader_impl impl, void *handle);

LOADER_API int loader_impl_handle_validate(void *handle);

LOADER_API value loader_impl_metadata(loader_impl impl);

LOADER_API int loader_impl_clear(void *handle);

LOADER_API void loader_impl_destroy_objects(loader_impl impl);

LOADER_API void loader_impl_destroy_deallocate(loader_impl impl);

LOADER_API void loader_impl_destroy_dtor(plugin p);

LOADER_API void loader_impl_destroy(plugin p, loader_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_IMPL_H */
