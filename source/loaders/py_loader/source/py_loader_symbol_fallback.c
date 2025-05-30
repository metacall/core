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

#include <Python.h>

/* Required for when linking to Python in debug mode and loading with Python.exe in release mode */
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	#if defined(__clang__) || defined(__GNUC__)

__attribute__((weak)) void _Py_DECREF_DecRefTotal(void) {}
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
