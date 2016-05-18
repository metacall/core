/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading python code at run-time into a process.
 *
 */

#include <py_loader/py_loader_impl.h>

#include <loader/loader_impl.h>

#include <stdlib.h>
#include <stdio.h>

#include <Python.h>

typedef struct loader_impl_py_type
{
	PyObject * inspect_module;
	PyObject * inspect_signature;

} * loader_impl_py;

int py_loader_impl_initialize_inspect(loader_impl_py py_impl)
{
	PyObject * module_name = PyUnicode_DecodeFSDefault("inspect");

	py_impl->inspect_module = PyImport_Import(module_name);

	Py_DECREF(module_name);

	if (py_impl->inspect_module != NULL)
	{
		py_impl->inspect_signature = PyObject_GetAttrString(py_impl->inspect_module, "signature");

		if (py_impl->inspect_signature != NULL && PyCallable_Check(py_impl->inspect_signature))
		{
			return 0;
		}

		Py_XDECREF(py_impl->inspect_signature);
	}

	Py_DECREF(py_impl->inspect_module);

	return 1;
}

loader_impl_data py_loader_impl_initialize(loader_impl impl)
{
	loader_impl_py py_impl = malloc(sizeof(struct loader_impl_py_type));

	if (py_impl != NULL)
	{
		Py_Initialize();

		if (py_loader_impl_initialize_inspect(py_impl) == 0)
		{
			if (py_loader_impl_execution_path(impl, ".") == 0)
			{
				return py_impl;
			}
		}
	}

	return NULL;
}

int py_loader_impl_execution_path(loader_impl impl, loader_naming_path path)
{
	loader_impl_py py_impl = loader_impl_get(impl);

	if (py_impl != NULL)
	{
		PyObject * system_path = PySys_GetObject("path");
		PyObject * current_path = PyUnicode_DecodeFSDefault(path);

		PyList_Append(system_path, current_path);

		Py_DECREF(current_path);

		return 0;
	}

	return 1;
}

loader_handle py_loader_impl_load(loader_impl impl, loader_naming_name name)
{
	PyObject * module_name = PyUnicode_DecodeFSDefault(name);

	PyObject * module = PyImport_Import(module_name);

	Py_DECREF(module_name);

	printf("Python loader (%p) importing %s module (%p)\n", (void *)impl, name, (void *)module);

	return (loader_handle)module;
}

int py_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	(void)impl;

	if (handle != NULL)
	{
		PyObject * module = (PyObject *)handle;

		Py_DECREF(module);

		return 0;
	}

	return 1;
}

int py_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	(void)impl; (void)handle; (void)ctx;




	return 0;
}

int py_loader_impl_destroy_inspect(loader_impl_py py_impl)
{
	Py_DECREF(py_impl->inspect_signature);

	Py_DECREF(py_impl->inspect_module);

	return 0;
}

int py_loader_impl_destroy(loader_impl impl)
{
	loader_impl_py py_impl = loader_impl_get(impl);

	if (py_impl != NULL)
	{
		if (py_loader_impl_destroy_inspect(py_impl) != 0)
		{
			/* error */
		}

		Py_Finalize();

		free(py_impl);

		return 0;
	}

	return 1;
}
