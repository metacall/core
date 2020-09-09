/*
 *	Loader Library by Parra Studios
 *	A plugin for loading python code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <py_loader/py_loader_api.h>

#include <reflect/reflect_type.h>

#include <loader/loader_impl.h>

#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

struct loader_impl_py_type;
typedef struct loader_impl_py_type * loader_impl_py;

PY_LOADER_NO_EXPORT type_id py_loader_impl_capi_to_value_type(PyObject * obj);

PY_LOADER_NO_EXPORT value py_loader_impl_capi_to_value(loader_impl impl, PyObject * obj, type_id id);

PY_LOADER_NO_EXPORT PyObject * py_loader_impl_value_to_capi(loader_impl impl, type_id id, value v);

#ifdef __cplusplus
}
#endif

#endif /* PY_LOADER_PORT_H */
