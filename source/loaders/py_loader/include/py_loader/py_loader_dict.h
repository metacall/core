/*
 *	Loader Library by Parra Studios
 *	A plugin for loading python code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef PY_LOADER_DICT_H
#define PY_LOADER_DICT_H 1

#include <py_loader/py_loader_api.h>

#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

PY_LOADER_NO_EXPORT int py_loader_impl_dict_type_init(void);

PY_LOADER_NO_EXPORT PyObject *py_loader_impl_finalizer_wrap_dict(PyObject *obj, void *v);

PY_LOADER_NO_EXPORT void py_loader_impl_dict_debug(PyObject *py_dict);

#ifdef __cplusplus
}
#endif

#endif /* PY_LOADER_DICT_H */
