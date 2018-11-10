/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#ifndef LOADER_PATH_H
#define LOADER_PATH_H 1

#include <loader/loader_api.h>

#include <loader/loader_naming.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

LOADER_API size_t loader_path_get_name(const loader_naming_path path, loader_naming_name name);

LOADER_API size_t loader_path_get_extension(const loader_naming_path path, loader_naming_tag extension);

LOADER_API size_t loader_path_get_path(const loader_naming_path path, size_t size, loader_naming_path absolute);

LOADER_API int loader_path_is_absolute(const loader_naming_path path);

LOADER_API size_t loader_path_join(const loader_naming_path left_path, size_t left_path_size, const loader_naming_path right_path, size_t right_path_size, loader_naming_path join_path);

LOADER_API size_t loader_path_canonical(const loader_naming_path path, size_t size, loader_naming_path canonical);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_PATH_H */
