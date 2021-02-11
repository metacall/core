/*
 *	Loader Library by Parra Studios
 *	A library for loading executable code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef LOADER_H
#define LOADER_H 1

/* -- Headers -- */

#include <loader/loader_api.h>

#include <loader/loader_naming.h>
#include <loader/loader_impl_interface.h>
#include <loader/loader_host.h>
#include <loader/loader_impl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Definitions -- */

#define LOADER_LOAD_FROM_FILES_SIZE 0x0400

/* -- Forward Declarations -- */

struct loader_type;

/* -- Type Definitions -- */

typedef value (*loader_register_invoke)(size_t, void * [], void *);

typedef void * loader_data;

typedef struct loader_type * loader;

/* -- Methods  -- */

LOADER_API loader loader_singleton(void);

LOADER_API void loader_copy(loader_host host);

LOADER_API void loader_initialization_register(loader_impl impl);

LOADER_API void loader_initialize(void);

LOADER_API int loader_is_initialized(const loader_naming_tag tag);

LOADER_API int loader_register(const char * name, loader_register_invoke invoke, function * func, type_id return_type, size_t arg_size, type_id args_type_id[]);

LOADER_API int loader_load_path(const loader_naming_path path);

LOADER_API int loader_execution_path(const loader_naming_tag tag, const loader_naming_path path);

LOADER_API int loader_load_from_file(const loader_naming_tag tag, const loader_naming_path paths[], size_t size, void ** handle);

LOADER_API int loader_load_from_memory(const loader_naming_tag tag, const char * buffer, size_t size, void ** handle);

LOADER_API int loader_load_from_package(const loader_naming_tag tag, const loader_naming_path path, void ** handle);

LOADER_API int loader_load_from_configuration(const loader_naming_path path, void ** handle, void * allocator);

LOADER_API loader_impl loader_get_impl(const loader_naming_tag tag);

LOADER_API loader_data loader_get(const char * name);

LOADER_API void * loader_get_handle(const loader_naming_tag tag, const char * name);

LOADER_API void loader_set_options(const loader_naming_tag tag, void * options);

LOADER_API void * loader_get_options(const loader_naming_tag tag);

LOADER_API const char * loader_handle_id(void * handle);

LOADER_API void * loader_handle_export(void * handle);

LOADER_API loader_data loader_handle_get(void * handle, const char * name);

LOADER_API value loader_metadata(void);

LOADER_API int loader_clear(void * handle);

LOADER_API void loader_unload_children(void);

LOADER_API int loader_unload(void);

LOADER_API void loader_destroy(void);

LOADER_API const char * loader_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_H */
