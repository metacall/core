/*
 *	Loader Library by Parra Studios
 *	A library for loading executable code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef LOADER_NAMING_H
#define LOADER_NAMING_H 1

#include <loader/loader_api.h>

#include <portability/portability_path.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOADER_PATH_SIZE PORTABILITY_PATH_SIZE
#define LOADER_NAME_SIZE 0xFF
#define LOADER_TAG_SIZE	 0x40

typedef char loader_path[LOADER_PATH_SIZE];
typedef char loader_name[LOADER_NAME_SIZE];
typedef char loader_tag[LOADER_TAG_SIZE];

#ifdef __cplusplus
}
#endif

#endif /* LOADER_NAMING_H */
