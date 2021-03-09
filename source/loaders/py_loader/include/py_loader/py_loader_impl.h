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

#ifndef PY_LOADER_IMPL_H
#define PY_LOADER_IMPL_H 1

#include <py_loader/py_loader_api.h>

#include <loader/loader_impl_interface.h>

#include <configuration/configuration.h>

#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

struct loader_impl_py_type;
typedef struct loader_impl_py_type * loader_impl_py;

PY_LOADER_API loader_impl_data py_loader_impl_initialize(loader_impl impl, configuration config);

PY_LOADER_API int py_loader_impl_execution_path(loader_impl impl, const loader_naming_path path);

PY_LOADER_API loader_handle py_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size);

PY_LOADER_API loader_handle py_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size);

PY_LOADER_API loader_handle py_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path);

PY_LOADER_API int py_loader_impl_clear(loader_impl impl, loader_handle handle);

PY_LOADER_API int py_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx);

PY_LOADER_API int py_loader_impl_destroy(loader_impl impl);

PY_LOADER_NO_EXPORT type_id py_loader_impl_capi_to_value_type(PyObject * obj);

PY_LOADER_NO_EXPORT value py_loader_impl_capi_to_value(loader_impl impl, PyObject * obj, type_id id);

PY_LOADER_NO_EXPORT PyObject * py_loader_impl_value_to_capi(loader_impl impl, type_id id, value v);

#ifdef __cplusplus
}
#endif

#endif /* PY_LOADER_IMPL_H */
