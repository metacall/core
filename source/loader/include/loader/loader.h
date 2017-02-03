/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#ifndef LOADER_H
#define LOADER_H 1

#include <loader/loader_api.h>

#include <loader/loader_naming.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#define LOADER_LOAD_FROM_FILES_SIZE 0x20

typedef void * loader_data;

LOADER_API void loader_initialize(void);

LOADER_API int loader_load_path(const loader_naming_path path);

LOADER_API int loader_load_from_file(const loader_naming_path paths[], size_t size);

LOADER_API int loader_load_from_memory(const loader_naming_extension extension, const char * buffer, size_t size);

LOADER_API int loader_load_from_package(const loader_naming_extension extension, const loader_naming_path path);

LOADER_API loader_data loader_get(const char * name);

LOADER_API int loader_unload(void);

LOADER_API void loader_destroy(void);

LOADER_API const char * loader_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_H */
