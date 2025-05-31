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

#include <py_loader/py_loader_symbol_fallback.h>

#include <dynlink/dynlink_type.h>

/* Required for Windows due to DELAYLOAD not supporting delayed import of data symbols */

#if defined(_WIN32) && defined(_MSC_VER)
static PyTypeObject *PyBool_TypePtr = NULL;
static PyTypeObject *PyFloat_TypePtr = NULL;
static PyTypeObject *PyCapsule_TypePtr = NULL;
static PyTypeObject *PyFunction_TypePtr = NULL;
static PyTypeObject *PyCFunction_TypePtr = NULL;
static PyTypeObject *PyStaticMethod_TypePtr = NULL;
static PyTypeObject *PyDictProxy_TypePtr = NULL;
static PyTypeObject *PyDict_TypePtr = NULL;
static PyTypeObject *PyModule_TypePtr = NULL;
static PyTypeObject *PyType_TypePtr = NULL;
static PyObject *Py_NoneStructPtr = NULL;
static PyObject *PyExc_ExceptionStructPtr = NULL;
static PyObject *PyExc_FileNotFoundErrorStructPtr = NULL;
static PyObject *PyExc_TypeErrorStructPtr = NULL;
static PyObject *PyExc_ValueErrorStructPtr = NULL;
static PyObject *PyExc_RuntimeErrorStructPtr = NULL;
static PyObject *Py_FalseStructPtr = NULL;
static PyObject *Py_TrueStructPtr = NULL;
#endif

int py_loader_symbol_fallback_initialize(dynlink py_library)
{
#if defined(_WIN32) && defined(_MSC_VER)
	dynlink_symbol_addr address;

	if (py_library == NULL)
	{
		return 1;
	}

	/* PyBool_Type */
	if (dynlink_symbol(py_library, "PyBool_Type", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyTypeObject *, PyBool_TypePtr);

	/* PyFloat_Type */
	if (dynlink_symbol(py_library, "PyFloat_Type", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyTypeObject *, PyFloat_TypePtr);

	/* PyCapsule_Type */
	if (dynlink_symbol(py_library, "PyCapsule_Type", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyTypeObject *, PyCapsule_TypePtr);

	/* PyFunction_Type */
	if (dynlink_symbol(py_library, "PyFunction_Type", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyTypeObject *, PyFunction_TypePtr);

	/* PyCFunction_Type */
	if (dynlink_symbol(py_library, "PyCFunction_Type", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyTypeObject *, PyCFunction_TypePtr);

	/* PyStaticMethod_Type */
	if (dynlink_symbol(py_library, "PyStaticMethod_Type", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyTypeObject *, PyStaticMethod_TypePtr);

	/* PyDict_TypePtr */
	if (dynlink_symbol(py_library, "PyDict_TypePtr", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyTypeObject *, PyDict_TypePtr);

	/* PyDictProxy_TypePtr */
	if (dynlink_symbol(py_library, "PyDictProxy_TypePtr", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyTypeObject *, PyDictProxy_TypePtr);

	/* PyModule_Type */
	if (dynlink_symbol(py_library, "PyModule_Type", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyTypeObject *, PyCFunction_TypePtr);

	/* PyType_Type */
	if (dynlink_symbol(py_library, "PyType_Type", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyTypeObject *, PyType_TypePtr);

	/* Py_None */
	if (dynlink_symbol(py_library, "_Py_NoneStruct", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyObject *, Py_NoneStructPtr);

	/* PyExc_Exception */
	if (dynlink_symbol(py_library, "PyExc_Exception", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyObject *, PyExc_ExceptionStructPtr);

	/* PyExc_FileNotFoundError */
	if (dynlink_symbol(py_library, "PyExc_FileNotFoundError", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyObject *, PyExc_FileNotFoundErrorStructPtr);

	/* PyExc_TypeError */
	if (dynlink_symbol(py_library, "PyExc_TypeError", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyObject *, PyExc_TypeErrorStructPtr);

	/* PyExc_ValueError */
	if (dynlink_symbol(py_library, "PyExc_ValueError", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyObject *, PyExc_ValueErrorStructPtr);

	/* PyExc_RuntimeError */
	if (dynlink_symbol(py_library, "PyExc_RuntimeError", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyObject *, PyExc_RuntimeErrorStructPtr);

	/* Py_False */
	if (dynlink_symbol(py_library, "_Py_FalseStruct", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyObject *, Py_FalseStructPtr);

	/* Py_True */
	if (dynlink_symbol(py_library, "_Py_TrueStruct", &address) != 0)
	{
		return 1;
	}

	dynlink_symbol_uncast_type(address, PyObject *, Py_TrueStructPtr);

	return 0;
#else
	(void)py_library;
	return 0;
#endif
}

#if defined(_WIN32) && defined(_MSC_VER)
int PyBool_Check(const PyObject *ob)
{
	return Py_IS_TYPE(ob, PyBool_TypePtr);
}

int PyFloat_Check(const PyObject *ob)
{
	return Py_IS_TYPE(ob, PyFloat_TypePtr);
}

int PyCapsule_CheckExact(const PyObject *ob)
{
	return Py_IS_TYPE(ob, PyCapsule_TypePtr);
}

int PyFunction_Check(const PyObject *ob)
{
	return Py_IS_TYPE(ob, PyFunction_TypePtr);
}

int PyCFunction_Check(const PyObject *ob)
{
	return Py_IS_TYPE(ob, PyCFunction_TypePtr);
}

int PyModule_Check(const PyObject *ob)
{
	return Py_IS_TYPE(ob, PyModule_TypePtr);
}
#endif

PyTypeObject *PyCFunctionTypePtr(void)
{
#if defined(_WIN32) && defined(_MSC_VER)
	return PyCFunction_TypePtr;
#else
	return &PyCFunction_Type;
#endif
}

PyTypeObject *PyStaticMethodTypePtr(void)
{
#if defined(_WIN32) && defined(_MSC_VER)
	return PyStaticMethod_TypePtr;
#else
	return &PyStaticMethod_Type;
#endif
}

PyTypeObject *PyDictTypePtr(void)
{
#if defined(_WIN32) && defined(_MSC_VER)
	return PyDict_TypePtr;
#else
	return &PyDict_Type;
#endif
}

PyTypeObject *PyDictProxyTypePtr(void)
{
#if defined(_WIN32) && defined(_MSC_VER)
	return PyDictProxy_TypePtr;
#else
	return &PyDictProxy_Type;
#endif
}

PyTypeObject *PyTypeTypePtr(void)
{
#if defined(_WIN32) && defined(_MSC_VER)
	return PyType_TypePtr;
#else
	return &PyType_Type;
#endif
}

PyObject *Py_NonePtr(void)
{
#if defined(_WIN32) && defined(_MSC_VER)
	return Py_NoneStructPtr;
#else
	return Py_None;
#endif
}

PyObject *PyExc_ExceptionPtr(void)
{
#if defined(_WIN32) && defined(_MSC_VER)
	return PyExc_ExceptionStructPtr;
#else
	return PyExc_Exception;
#endif
}

PyObject *PyExc_FileNotFoundErrorPtr(void)
{
#if defined(_WIN32) && defined(_MSC_VER)
	return PyExc_FileNotFoundErrorStructPtr;
#else
	return PyExc_FileNotFoundError;
#endif
}

PyObject *PyExc_TypeErrorPtr(void)
{
#if defined(_WIN32) && defined(_MSC_VER)
	return PyExc_TypeErrorStructPtr;
#else
	return PyExc_TypeError;
#endif
}

PyObject *PyExc_ValueErrorPtr(void)
{
#if defined(_WIN32) && defined(_MSC_VER)
	return PyExc_ValueErrorStructPtr;
#else
	return PyExc_ValueError;
#endif
}

PyObject *PyExc_RuntimeErrorPtr(void)
{
#if defined(_WIN32) && defined(_MSC_VER)
	return PyExc_RuntimeErrorStructPtr;
#else
	return PyExc_RuntimeError;
#endif
}

PyObject *Py_ReturnNone(void)
{
#if defined(_WIN32) && defined(_MSC_VER)
	Py_IncRef(Py_NoneStructPtr);
	return Py_NoneStructPtr;
#else
	Py_RETURN_NONE;
#endif
}

#if defined(__clang__)
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wstrict-aliasing"
#elif defined(__GNUC__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#elif defined(_MSC_VER)
	#pragma warning(push)
// TODO
#endif

PyObject *Py_ReturnFalse(void)
{
#if defined(_WIN32) && defined(_MSC_VER)
	Py_IncRef(Py_FalseStructPtr);
	return Py_FalseStructPtr;
#else
	Py_RETURN_FALSE;
#endif
}

PyObject *Py_ReturnTrue(void)
{
#if defined(_WIN32) && defined(_MSC_VER)
	Py_IncRef(Py_TrueStructPtr);
	return Py_TrueStructPtr;
#else
	Py_RETURN_TRUE;
#endif
}

#if defined(__clang__)
	#pragma clang diagnostic pop
#elif defined(__GNUC__)
	#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
	#pragma warning(pop)
#endif

/* Required on GNU for when linking to Python in debug mode and loading with Python.elf in release mode */
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	#if defined(__clang__) || defined(__GNUC__)

__attribute__((weak)) void _Py_DECREF_DecRefTotal(void)
{
}
__attribute__((weak)) void _Py_INCREF_IncRefTotal(void) {}
__attribute__((weak)) Py_ssize_t _Py_RefTotal;

		/* When Python has been compiled with tracing reference counting,
		* provide fallback symbols for allowing it to compile properly */
		#ifdef Py_TRACE_REFS

			#include <dynlink/dynlink.h>

			#undef PyModule_Create2
			#undef PyModule_FromDefAndSpec2

static dynlink_symbol_addr py_loader_symbol(const char *name)
{
	dynlink proc = dynlink_load_self(DYNLINK_FLAGS_BIND_NOW | DYNLINK_FLAGS_BIND_GLOBAL);
	dynlink_symbol_addr addr = NULL;

	if (proc == NULL)
	{
		return NULL;
	}

	dynlink_symbol(proc, name, &addr);

	dynlink_unload(proc);

	return addr;
}

__attribute__((weak)) PyObject *PyModule_Create2(struct PyModuleDef *module, int module_api_version)
{
	static PyObject *(*py_module_create2)(struct PyModuleDef *, int) = NULL;

	if (py_module_create2 == NULL)
	{
		py_module_create2 = (PyObject * (*)(struct PyModuleDef *, int)) py_loader_symbol("PyModule_Create2TraceRefs");
	}

	if (py_module_create2 == NULL)
	{
		return NULL;
	}

	return py_module_create2(module, module_api_version);
}
__attribute__((weak)) PyObject *PyModule_FromDefAndSpec2(PyModuleDef *def, PyObject *spec, int module_api_version)
{
	static PyObject *(*py_module_from_def_and_spec2)(struct PyModuleDef *, PyObject *, int) = NULL;

	if (py_module_from_def_and_spec2 == NULL)
	{
		py_module_from_def_and_spec2 = (PyObject * (*)(struct PyModuleDef *, PyObject *, int)) py_loader_symbol("PyModule_FromDefAndSpec2TraceRefs");
	}

	if (py_module_from_def_and_spec2 == NULL)
	{
		return NULL;
	}

	return py_module_from_def_and_spec2(def, spec, module_api_version);
}

		#endif
	#endif
#endif
