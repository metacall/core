/*
 *	Loader Library by Parra Studios
 *	A library for loading executable code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef LOADER_PATH_H
#define LOADER_PATH_H 1

#include <loader/loader_api.h>

#include <loader/loader_naming.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

LOADER_API size_t loader_path_get_name(const loader_naming_path path, loader_naming_name name);

LOADER_API size_t loader_path_get_fullname(const loader_naming_path path, loader_naming_name name);

LOADER_API size_t loader_path_get_extension(const loader_naming_path path, loader_naming_tag extension);

LOADER_API size_t loader_path_get_path(const loader_naming_path path, size_t size, loader_naming_path absolute);

LOADER_API int loader_path_is_absolute(const loader_naming_path path);

LOADER_API size_t loader_path_join(const loader_naming_path left_path, size_t left_path_size, const loader_naming_path right_path, size_t right_path_size, loader_naming_path join_path);

LOADER_API size_t loader_path_canonical(const loader_naming_path path, size_t size, loader_naming_path canonical);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_PATH_H */
