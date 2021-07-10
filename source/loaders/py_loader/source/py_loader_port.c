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

#include <metacall/metacall.h>

#include <py_loader/py_loader_impl.h>
#include <py_loader/py_loader_port.h>

#include <loader/loader.h>

#ifndef PY_LOADER_PORT_NAME
	#error "The Python Loader Port must be defined"
#endif

static PyObject *py_loader_port_none()
{
	Py_RETURN_NONE;
}

static PyObject *py_loader_port_false()
{
	Py_RETURN_FALSE;
}

static PyObject *py_loader_port_true()
{
	Py_RETURN_TRUE;
}

static PyObject *py_loader_port_load_from_file(PyObject *self, PyObject *args)
{
	static const char format[] = "OO:metacall_load_from_file";
	PyObject *tag, *paths, *result = NULL;
	char *tag_str, **paths_str;
	Py_ssize_t tag_length = 0;
	size_t paths_size, iterator, alloc_iterator;

	(void)self;

	/* Parse arguments */
	if (!PyArg_ParseTuple(args, (char *)format, &tag, &paths))
	{
		PyErr_SetString(PyExc_TypeError, "Invalid number of arguments, use it like: metacall_load_from_file('node', ['script.js']);");
		return py_loader_port_false();
	}

#if PY_MAJOR_VERSION == 2
	if (!PyString_Check(tag))
#elif PY_MAJOR_VERSION == 3
	if (!PyUnicode_Check(tag))
#endif
	{
		PyErr_SetString(PyExc_TypeError, "Invalid parameter type in first argument (a string indicating the tag of the loader must be used: 'node', 'rb', 'ts', 'cs', 'js', 'cob'...)");
		return py_loader_port_false();
	}

	if (!PyList_Check(paths))
	{
		PyErr_SetString(PyExc_TypeError, "Invalid parameter type in second argument (a list of strings indicating the paths must be used)");
		return py_loader_port_false();
	}

	paths_size = PyList_Size(paths);

	if (paths_size == 0)
	{
		PyErr_SetString(PyExc_TypeError, "At least one path must be included in the paths list");
		return py_loader_port_false();
	}

/* Convert tag from unicode into a string */
#if PY_MAJOR_VERSION == 2
	{
		if (PyString_AsStringAndSize(tag, &tag_str, &tag_length) == -1)
		{
			tag_str = NULL;
		}
	}
#elif PY_MAJOR_VERSION == 3
	{
		tag_str = (char *)PyUnicode_AsUTF8AndSize(tag, &tag_length);
	}
#endif

	if (tag_str == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "Invalid tag string conversion");
		return py_loader_port_false();
	}

	/* Convert paths list into an array of strings */
	paths_str = (char **)malloc(sizeof(char *) * paths_size);

	if (paths_str == NULL)
	{
		PyErr_SetString(PyExc_ValueError, "Invalid argument allocation");
		return py_loader_port_false();
	}

	for (iterator = 0; iterator < paths_size; ++iterator)
	{
		PyObject *path = PyList_GetItem(paths, iterator);

		int check_path =
#if PY_MAJOR_VERSION == 2
			PyString_Check(path);
#elif PY_MAJOR_VERSION == 3
			PyUnicode_Check(path);
#endif

		if (check_path != 0)
		{
			char *str = NULL;
			Py_ssize_t length = 0;

#if PY_MAJOR_VERSION == 2
			{
				if (PyString_AsStringAndSize(path, &str, &length) == -1)
				{
					str = NULL;
				}
			}
#elif PY_MAJOR_VERSION == 3
			{
				str = (char *)PyUnicode_AsUTF8AndSize(path, &length);
			}
#endif

			if (str == NULL)
			{
				PyErr_SetString(PyExc_TypeError, "Invalid path string conversion");
				result = py_loader_port_false();
				goto clear;
			}

			paths_str[iterator] = (char *)malloc(sizeof(char) * (length + 1));

			if (paths_str[iterator] == NULL)
			{
				PyErr_SetString(PyExc_ValueError, "Invalid string path allocation");
				result = py_loader_port_false();
				goto clear;
			}

			memcpy(paths_str[iterator], str, length);

			paths_str[iterator][length] = '\0';
		}
	}

	/* Execute the load from file call */
	{
		PyThreadState *thread_state = PyEval_SaveThread();

		int ret = metacall_load_from_file(tag_str, (const char **)paths_str, paths_size, NULL);

		PyEval_RestoreThread(thread_state);

		if (ret != 0)
		{
			result = py_loader_port_false();
			goto clear;
		}
	}

	result = py_loader_port_true();

clear:
	for (alloc_iterator = 0; alloc_iterator < iterator; ++alloc_iterator)
	{
		free(paths_str[alloc_iterator]);
	}

	free(paths_str);

	return result;
}

static PyObject *py_loader_port_load_from_memory(PyObject *self, PyObject *args)
{
	static const char format[] = "OO:metacall_load_from_memory";
	PyObject *tag, *buffer;
	char *tag_str, *buffer_str;
	Py_ssize_t buffer_length = 0, tag_length = 0;

	(void)self;

	/* Parse arguments */
	if (!PyArg_ParseTuple(args, (char *)format, &tag, &buffer))
	{
		PyErr_SetString(PyExc_TypeError, "Invalid number of arguments, use it like: metacall_load_from_memory('node', 'console.log(\"hello\")');");
		return py_loader_port_false();
	}

#if PY_MAJOR_VERSION == 2
	if (!PyString_Check(tag))
#elif PY_MAJOR_VERSION == 3
	if (!PyUnicode_Check(tag))
#endif
	{
		PyErr_SetString(PyExc_TypeError, "Invalid parameter type in first argument (a string indicating the tag of the loader must be used: 'node', 'rb', 'ts', 'cs', 'js', 'cob'...)");
		return py_loader_port_false();
	}

#if PY_MAJOR_VERSION == 2
	if (!PyString_Check(buffer))
#elif PY_MAJOR_VERSION == 3
	if (!PyUnicode_Check(buffer))
#endif
	{
		PyErr_SetString(PyExc_TypeError, "Invalid parameter type in second argument (a string indicating the tag of the loader must be used: 'console.log(\"hello\")')");
		return py_loader_port_false();
	}

/* Convert tag from unicode into a string */
#if PY_MAJOR_VERSION == 2
	{
		if (PyString_AsStringAndSize(tag, &tag_str, &tag_length) == -1)
		{
			tag_str = NULL;
		}
	}
#elif PY_MAJOR_VERSION == 3
	{
		tag_str = (char *)PyUnicode_AsUTF8AndSize(tag, &tag_length);
	}
#endif

	if (tag_str == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "Invalid tag string conversion");
		return py_loader_port_false();
	}

/* Convert buffer from unicode into a string */
#if PY_MAJOR_VERSION == 2
	{
		if (PyString_AsStringAndSize(buffer, &buffer_str, &buffer_length) == -1)
		{
			buffer_str = NULL;
		}
	}
#elif PY_MAJOR_VERSION == 3
	{
		buffer_str = (char *)PyUnicode_AsUTF8AndSize(buffer, &buffer_length);
	}
#endif

	if (buffer_str == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "Invalid buffer string conversion");
		return py_loader_port_false();
	}

	/* Execute the load from memory call */
	{
		PyThreadState *thread_state = PyEval_SaveThread();

		int ret = metacall_load_from_memory(tag_str, (const char *)buffer_str, buffer_length + 1, NULL);

		PyEval_RestoreThread(thread_state);

		if (ret != 0)
		{
			return py_loader_port_false();
		}
	}

	return py_loader_port_true();
}

static PyObject *py_loader_port_invoke(PyObject *self, PyObject *var_args)
{
	PyObject *name, *result = NULL;
	char *name_str;
	Py_ssize_t name_length = 0;
	void **value_args = NULL;
	size_t args_size = 0, args_count;
	Py_ssize_t var_args_size;
	loader_impl impl;

	(void)self;

	/* Obtain Python loader implementation */
	impl = loader_get_impl("py");

	if (impl == NULL)
	{
		PyErr_SetString(PyExc_ValueError, "Invalid Python loader instance, MetaCall Port must be used from MetaCall CLI");
		return py_loader_port_none();
	}

	var_args_size = PyTuple_Size(var_args);

	if (var_args_size == 0)
	{
		PyErr_SetString(PyExc_TypeError, "Invalid number of arguments, use it like: metacall('function_name', 'asd', 123, [7, 4]);");
		return py_loader_port_none();
	}

	name = PyTuple_GetItem(var_args, 0);

#if PY_MAJOR_VERSION == 2
	{
		if (!(PyString_Check(name) && PyString_AsStringAndSize(name, &name_str, &name_length) != -1))
		{
			name_str = NULL;
		}
	}
#elif PY_MAJOR_VERSION == 3
	{
		name_str = PyUnicode_Check(name) ? (char *)PyUnicode_AsUTF8AndSize(name, &name_length) : NULL;
	}
#endif

	if (name_str == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "Invalid function name string conversion, first parameter must be a string");
		return py_loader_port_none();
	}

	/* Get variable arguments length */
	args_size = var_args_size - 1;

	/* Allocate arguments */
	if (args_size != 0)
	{
		value_args = (void **)malloc(args_size * sizeof(void *));

		if (value_args == NULL)
		{
			PyErr_SetString(PyExc_ValueError, "Invalid argument allocation");
			return py_loader_port_none();
		}

		/* Parse variable arguments */
		for (args_count = 0; args_count < args_size; ++args_count)
		{
			PyObject *element = PyTuple_GetItem(var_args, args_count + 1);

			value_args[args_count] = py_loader_impl_capi_to_value(impl, element, py_loader_impl_capi_to_value_type(impl, element));
		}
	}

	/* Execute the invocation */
	{
		PyThreadState *thread_state = PyEval_SaveThread();

		void *ret;

		if (value_args != NULL)
		{
			ret = metacallv_s(name_str, value_args, args_size);
		}
		else
		{
			ret = metacallv_s(name_str, metacall_null_args, 0);
		}

		PyEval_RestoreThread(thread_state);

		if (ret == NULL)
		{
			result = py_loader_port_none();
			goto clear;
		}

		result = py_loader_impl_value_to_capi(impl, value_type_id(ret), ret);

		value_type_destroy(ret);

		if (result == NULL)
		{
			result = py_loader_port_none();
			goto clear;
		}
	}

clear:
	if (value_args != NULL)
	{
		for (args_count = 0; args_count < args_size; ++args_count)
		{
			value_type_destroy(value_args[args_count]);
		}

		free(value_args);
	}

	return result;
}

static PyObject *py_loader_port_inspect(PyObject *self, PyObject *args)
{
	PyObject *result = NULL;
	size_t size = 0;
	char *result_str = NULL, *inspect_str = NULL;
	struct metacall_allocator_std_type std_ctx = { &malloc, &realloc, &free };

	/* Create the allocator */
	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	(void)self;
	(void)args;

	/* Retrieve inspect data */
	result_str = inspect_str = metacall_inspect(&size, allocator);

	if (inspect_str == NULL || size == 0)
	{
		static const char empty[] = "{}";

		result_str = (char *)empty;
		size = sizeof(empty);

		PyErr_SetString(PyExc_ValueError, "Inspect returned an invalid size or string");
	}

#if PY_MAJOR_VERSION == 2
	result = PyString_FromStringAndSize(result_str, (Py_ssize_t)(size - 1));
#elif PY_MAJOR_VERSION == 3
	result = PyUnicode_FromStringAndSize(result_str, (Py_ssize_t)(size - 1));
#endif

	if (inspect_str != NULL)
	{
		metacall_allocator_free(allocator, inspect_str);
	}

	metacall_allocator_destroy(allocator);

	return result;
}

static PyMethodDef metacall_methods[] = {
	{ "metacall_load_from_file", py_loader_port_load_from_file, METH_VARARGS,
		"Loads a script from file." },
	{ "metacall_load_from_memory", py_loader_port_load_from_memory, METH_VARARGS,
		"Loads a script from a string." },
	{ "metacall_inspect", py_loader_port_inspect, METH_NOARGS,
		"Get information about all loaded objects." },
	{ "metacall", py_loader_port_invoke, METH_VARARGS,
		"Call a function anonymously." },
	{ NULL, NULL, 0, NULL }
};

static struct PyModuleDef metacall_definition = {
	PyModuleDef_HEAD_INIT,
	"metacall",
	"A library for providing inter-language foreign function interface calls.",
	-1,
	metacall_methods,
	NULL,
	NULL,
	NULL,
	NULL
};

PyMODINIT_FUNC PY_LOADER_PORT_NAME_FUNC()
{
	static PyObject *module = NULL;

	if (module == NULL)
	{
		module = PyModule_Create(&metacall_definition);
	}

	return module;
}
