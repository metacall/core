/*
 *	Loader Library by Parra Studios
 *	A plugin for loading python code at run-time into a process.
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

#ifndef PY_LOADER_PORT_H
#define PY_LOADER_PORT_H 1

#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PY_LOADER_PORT_NAME_FUNC_IMPL_EXPAND(x) PyInit_##x
#define PY_LOADER_PORT_NAME_FUNC_IMPL(x)		PY_LOADER_PORT_NAME_FUNC_IMPL_EXPAND(x)
#define PY_LOADER_PORT_NAME_FUNC				PY_LOADER_PORT_NAME_FUNC_IMPL(PY_LOADER_PORT_NAME)

PyMODINIT_FUNC PY_LOADER_PORT_NAME_FUNC(void);

#ifdef __cplusplus
}
#endif

#endif /* PY_LOADER_PORT_H */
