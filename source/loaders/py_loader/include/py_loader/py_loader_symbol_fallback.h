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

#ifndef PY_LOADER_SYMBOL_FALLBACK_H
#define PY_LOADER_SYMBOL_FALLBACK_H 1

#include <py_loader/py_loader_api.h>

#include <dynlink/dynlink.h>

#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

PY_LOADER_NO_EXPORT int py_loader_symbol_fallback_initialize(dynlink py_library);

#if defined(_WIN32) && defined(_MSC_VER)
	#undef PyBool_Check
PY_LOADER_NO_EXPORT int PyBool_Check(const PyObject *ob);
	#undef PyFloat_Check
PY_LOADER_NO_EXPORT int PyFloat_Check(const PyObject *ob);
	#undef PyCapsule_CheckExact
PY_LOADER_NO_EXPORT int PyCapsule_CheckExact(const PyObject *ob);
	#undef PyFunction_Check
PY_LOADER_NO_EXPORT int PyFunction_Check(const PyObject *ob);
	#undef PyCFunction_Check
PY_LOADER_NO_EXPORT int PyCFunction_Check(const PyObject *ob);
	#undef PyModule_Check
PY_LOADER_NO_EXPORT int PyModule_Check(const PyObject *ob);
#endif

PY_LOADER_NO_EXPORT PyTypeObject *PyTypeTypePtr(void);
PY_LOADER_NO_EXPORT PyObject *Py_NonePtr(void);
PY_LOADER_NO_EXPORT PyObject *Py_ReturnNone(void);
PY_LOADER_NO_EXPORT PyObject *Py_ReturnFalse(void);
PY_LOADER_NO_EXPORT PyObject *Py_ReturnTrue(void);

#ifdef __cplusplus
}
#endif

#endif /* PY_LOADER_SYMBOL_FALLBACK_H */
