/*
 *	Loader Library by Parra Studios
 *	A plugin for loading crystal code at run-time into a process.
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

#ifndef CR_LOADER_IMPL_MAIN_H
#define CR_LOADER_IMPL_MAIN_H 1

#include <cr_loader/cr_loader_api.h>

#ifdef __cplusplus
extern "C" {
#endif

CR_LOADER_API void crystal_library_init(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif /* CR_LOADER_IMPL_MAIN_H */
