/*
 *	Loader Library by Parra Studios
 *	A plugin for loading python code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef PY_LOADER_OBJECT_H
#define PY_LOADER_OBJECT_H 1

#include <py_loader/py_loader_api.h>

#include <py_loader/py_loader_impl.h>

#include <Python.h>

/* Vectorcall and method descriptors are available since Python 3.8 */
#if PY_VERSION_HEX >= 0x03080000
	#define PY_LOADER_IMPL_VECTORCALL 1
	#ifndef Py_TPFLAGS_HAVE_VECTORCALL
		#define Py_TPFLAGS_HAVE_VECTORCALL _Py_TPFLAGS_HAVE_VECTORCALL
	#endif
#else
	#define PY_LOADER_IMPL_VECTORCALL 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Same signature as metacallv_class and metacallv_object */
typedef void *(*py_loader_impl_reflect_invoke_ptr)(void *, const char *, void *[], size_t);

PY_LOADER_NO_EXPORT int py_loader_impl_object_type_init(void);

PY_LOADER_NO_EXPORT void py_loader_impl_object_type_destroy(void);

PY_LOADER_NO_EXPORT PyTypeObject *py_loader_impl_object_base_type(void);

PY_LOADER_NO_EXPORT int py_loader_impl_object_check(PyObject *obj);

PY_LOADER_NO_EXPORT void *py_loader_impl_object_copy(PyObject *obj);

PY_LOADER_NO_EXPORT PyObject *py_loader_impl_object_new(loader_impl impl, value obj);

PY_LOADER_NO_EXPORT PyObject *py_loader_impl_object_wrap(PyTypeObject *type, loader_impl impl, value obj);

PY_LOADER_NO_EXPORT PyObject *py_loader_impl_object_attribute_new(PyObject *name);

PY_LOADER_NO_EXPORT PyObject *py_loader_impl_object_method_new(PyObject *name);

PY_LOADER_NO_EXPORT PyObject *py_loader_impl_method_invoke(loader_impl impl, void *target, PyObject *name, py_loader_impl_reflect_invoke_ptr invoke, PyObject *const *args, size_t argc);

#ifdef __cplusplus
}
#endif

#endif /* PY_LOADER_OBJECT_H */
