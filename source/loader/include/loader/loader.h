/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#ifndef LOADER_H
#define LOADER_H 1

/* -- Headers -- */

#include <loader/loader_api.h>

#include <loader/loader_naming.h>
#include <loader/loader_impl_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Definitions -- */

#define LOADER_LOAD_FROM_FILES_SIZE 0x40

/* -- Forward Declarations -- */

struct loader_type;

/* -- Type Definitions -- */

typedef value (*loader_register_invoke)(void * []);

typedef void * loader_data;

typedef struct loader_type * loader;

/* -- Methods  -- */

LOADER_API loader loader_singleton(void);

LOADER_API void loader_initialize(void);

LOADER_API int loader_register(const char * name, loader_register_invoke invoke, type_id return_type, size_t arg_size, type_id args_type_id[]);

LOADER_API int loader_load_path(const loader_naming_path path);

LOADER_API int loader_execution_path(const loader_naming_tag tag, const loader_naming_path path);

LOADER_API int loader_load_from_file(const loader_naming_tag tag, const loader_naming_path paths[], size_t size, void ** handle);

LOADER_API int loader_load_from_memory(const loader_naming_tag tag, const char * buffer, size_t size, void ** handle);

LOADER_API int loader_load_from_package(const loader_naming_tag tag, const loader_naming_path path, void ** handle);

LOADER_API int loader_load_from_configuration(const loader_naming_path path, void ** handle, void * allocator);

LOADER_API loader_impl loader_get_impl(const loader_naming_tag tag);

LOADER_API loader_data loader_get(const char * name);

LOADER_API void * loader_get_handle(const loader_naming_tag tag, const char * name);

LOADER_API const char * loader_handle_id(void * handle);

LOADER_API value loader_metadata(void);

LOADER_API int loader_clear(void * handle);

LOADER_API int loader_unload(void);

LOADER_API void loader_destroy(void);

LOADER_API const char * loader_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_H */
