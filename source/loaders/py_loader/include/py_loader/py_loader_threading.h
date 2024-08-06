/*
 *	Loader Library by Parra Studios
 *	A plugin for loading python code at run-time into a process.
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

#ifndef PY_LOADER_THREADING_H
#define PY_LOADER_THREADING_H 1

#include <py_loader/py_loader_api.h>

#ifdef __cplusplus
extern "C" {
#endif

PY_LOADER_NO_EXPORT void py_loader_thread_initialize(void);

PY_LOADER_NO_EXPORT int py_loader_thread_is_main(void);

PY_LOADER_NO_EXPORT void py_loader_thread_acquire(void);

PY_LOADER_NO_EXPORT void py_loader_thread_release(void);

#ifdef __cplusplus
}
#endif

#endif /* PY_LOADER_THREADING_H */
