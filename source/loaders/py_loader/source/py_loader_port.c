/*
 *	Loader Library by Parra Studios
 *	A plugin for loading python code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <py_loader/py_loader_dict.h>
#include <py_loader/py_loader_impl.h>
#include <py_loader/py_loader_port.h>
#include <py_loader/py_loader_threading.h>

#include <loader/loader.h>

#ifndef PY_LOADER_PORT_NAME
	#error "The Python Loader Port must be defined"
#endif

static const loader_tag py_loader_tag = "py";

static PyObject *py_loader_port_none(void)
{
	Py_RETURN_NONE;
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

static PyObject *py_loader_port_false(void)
{
	Py_RETURN_FALSE;
}

static PyObject *py_loader_port_true(void)
{
	Py_RETURN_TRUE;
}

#if defined(__clang__)
	#pragma clang diagnostic pop
#elif defined(__GNUC__)
	#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
	#pragma warning(pop)
#endif

static PyObject *py_loader_port_load_from_file_impl(PyObject *self, PyObject *args, void **handle)
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

	py_loader_thread_release();

	/* Execute the load from file call */
	int ret = metacall_load_from_file(tag_str, (const char **)paths_str, paths_size, handle);

	py_loader_thread_acquire();

	if (ret != 0)
	{
		result = handle == NULL ? py_loader_port_false() : py_loader_port_none();
		goto clear;
	}
	else
	{
		if (handle != NULL)
		{
			loader_impl impl = loader_get_impl(py_loader_tag);

			void *exports = metacall_handle_export(*handle);

			result = py_loader_impl_value_to_capi(impl, value_type_id(exports), exports);

			PyObject *wrapper = py_loader_impl_finalizer_wrap_map(result, exports);

			if (wrapper == NULL)
			{
				Py_XDECREF(result);
				result = py_loader_port_none();
			}
			else
			{
				result = wrapper;
			}
		}
		else
		{
			result = py_loader_port_true();
		}
	}

clear:
	for (alloc_iterator = 0; alloc_iterator < iterator; ++alloc_iterator)
	{
		free(paths_str[alloc_iterator]);
	}

	free(paths_str);

	return result;
}

static PyObject *py_loader_port_load_from_file(PyObject *self, PyObject *args)
{
	return py_loader_port_load_from_file_impl(self, args, NULL);
}

static PyObject *py_loader_port_load_from_file_export(PyObject *self, PyObject *args)
{
	void *handle = NULL;
	return py_loader_port_load_from_file_impl(self, args, &handle);
}

static PyObject *py_loader_port_load_from_package_impl(PyObject *self, PyObject *args, void **handle)
{
	static const char format[] = "OO:metacall_load_from_package";
	PyObject *tag, *path, *result = NULL;
	char *tag_str, *path_str;
	Py_ssize_t tag_length = 0;

	(void)self;

	/* Parse arguments */
	if (!PyArg_ParseTuple(args, (char *)format, &tag, &path))
	{
		PyErr_SetString(PyExc_TypeError, "Invalid number of arguments, use it like: metacall_load_from_package('cs', ['file.dll']);");
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
	if (!PyString_Check(path))
#elif PY_MAJOR_VERSION == 3
	if (!PyUnicode_Check(path))
#endif
	{
		PyErr_SetString(PyExc_TypeError, "Invalid parameter type in second argument (a string indicating the path must be used)");
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

#if PY_MAJOR_VERSION == 2
	PyString_Check(path);
#elif PY_MAJOR_VERSION == 3
	PyUnicode_Check(path);
#endif

	Py_ssize_t length = 0;
#if PY_MAJOR_VERSION == 2
	{
		if (PyString_AsStringAndSize(path, &path_str, &length) == -1)
		{
			path_str = NULL;
		}
	}
#elif PY_MAJOR_VERSION == 3
	{
		path_str = (char *)PyUnicode_AsUTF8AndSize(path, &length);
	}
#endif

	if (path_str == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "Invalid path string conversion");
		return py_loader_port_false();
	}

	py_loader_thread_release();

	/* Execute the load from package call */
	int ret = metacall_load_from_package(tag_str, path_str, handle);

	py_loader_thread_acquire();

	if (ret != 0)
	{
		result = handle == NULL ? py_loader_port_false() : py_loader_port_none();
	}
	else
	{
		if (handle != NULL)
		{
			loader_impl impl = loader_get_impl(py_loader_tag);

			void *exports = metacall_handle_export(*handle);

			result = py_loader_impl_value_to_capi(impl, value_type_id(exports), exports);

			PyObject *wrapper = py_loader_impl_finalizer_wrap_map(result, exports);

			if (wrapper == NULL)
			{
				Py_XDECREF(result);
				result = py_loader_port_none();
			}
			else
			{
				result = wrapper;
			}
		}
		else
		{
			result = py_loader_port_true();
		}
	}

	return result;
}

static PyObject *py_loader_port_load_from_package(PyObject *self, PyObject *args)
{
	return py_loader_port_load_from_package_impl(self, args, NULL);
}

static PyObject *py_loader_port_load_from_package_export(PyObject *self, PyObject *args)
{
	void *handle = NULL;
	return py_loader_port_load_from_package_impl(self, args, &handle);
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
		py_loader_thread_release();

		int ret = metacall_load_from_memory(tag_str, (const char *)buffer_str, buffer_length + 1, NULL);

		py_loader_thread_acquire();

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
	impl = loader_get_impl(py_loader_tag);

	/* TODO: Remove this check when we implement this: https://github.com/metacall/core/issues/231 */
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
		void *ret;

		py_loader_thread_release();

		if (value_args != NULL)
		{
			ret = metacallv_s(name_str, value_args, args_size);
		}
		else
		{
			ret = metacallv_s(name_str, metacall_null_args, 0);
		}

		py_loader_thread_acquire();

		if (ret == NULL)
		{
			result = py_loader_port_none();
			goto clear;
		}

		result = py_loader_impl_value_to_capi(impl, value_type_id(ret), ret);

		py_loader_thread_release();
		value_type_destroy(ret);
		py_loader_thread_acquire();

		if (result == NULL)
		{
			result = py_loader_port_none();
			goto clear;
		}
	}

clear:
	if (value_args != NULL)
	{
		py_loader_thread_release();

		for (args_count = 0; args_count < args_size; ++args_count)
		{
			value_type_destroy(value_args[args_count]);
		}

		py_loader_thread_acquire();

		free(value_args);
	}

	return result;
}

// TODO
#if 0
static PyObject *py_loader_port_await(PyObject *self, PyObject *var_args)
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
	impl = loader_get_impl(py_loader_tag);

	/* TODO: Remove this check when we implement this: https://github.com/metacall/core/issues/231 */
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

	/* Execute the await */
	{
		void *ret;

		py_loader_thread_release();

		/* TODO: */
		/*
		if (value_args != NULL)
		{
			ret = metacallv_s(name_str, value_args, args_size);
		}
		else
		{
			ret = metacallv_s(name_str, metacall_null_args, 0);
		}
		*/

		py_loader_thread_acquire();

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
#endif

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

	py_loader_thread_release();

	/* Retrieve inspect data */
	result_str = inspect_str = metacall_inspect(&size, allocator);

	py_loader_thread_acquire();

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

static PyObject *py_loader_port_value_create_ptr(PyObject *self, PyObject *args)
{
	static const char format[] = "O:metacall_value_create_ptr";
	PyObject *pointer;

	(void)self;

	/* Parse arguments */
	if (!PyArg_ParseTuple(args, (char *)format, &pointer))
	{
		PyErr_SetString(PyExc_TypeError, "Invalid number of arguments, use it like: metacall_value_create_ptr(None); or metacall_value_create_ptr(previous_allocated_ptr);");
		return py_loader_port_none();
	}

	if (!PyCapsule_CheckExact(pointer) && pointer != Py_None)
	{
		PyErr_SetString(PyExc_TypeError, "Invalid parameter type in first argument must be None or a PyCapsule (i.e a previously allocated pointer)");
		return py_loader_port_none();
	}

	if (pointer == Py_None)
	{
		return py_loader_impl_capsule_new_null();
	}
	else
	{
		/* Get capsule pointer */
		const char *name = PyCapsule_GetName(pointer);
		void *pointer_addr = PyCapsule_GetPointer(pointer, name);

		/* Return a copy of the capsule */
		return PyCapsule_New(pointer_addr, name, NULL);
	}
}

static const char py_loader_capsule_reference_id[] = "__metacall_capsule_reference__";

static void py_loader_port_value_reference_destroy(PyObject *capsule)
{
	void *v = PyCapsule_GetPointer(capsule, py_loader_capsule_reference_id);

	metacall_value_destroy(v);
}

static PyObject *py_loader_port_value_reference(PyObject *self, PyObject *args)
{
	static const char format[] = "O:metacall_value_reference";
	PyObject *obj;
	loader_impl impl;
	void *v;
	PyObject *capsule;

	(void)self;

	/* Parse arguments */
	if (!PyArg_ParseTuple(args, (char *)format, &obj))
	{
		PyErr_SetString(PyExc_TypeError, "Invalid number of arguments, use it like: metacall_value_reference(obj);");
		goto error_none;
	}

	/* Obtain Python loader implementation */
	impl = loader_get_impl(py_loader_tag);

	/* TODO: When using the port outside MetaCall this is going to segfault for functions and similar
	* structures that require py loader internal structure to be initialized. For those cases, we
	* must implement this: https://github.com/metacall/core/issues/231
	*/
	v = py_loader_impl_capi_to_value(impl, obj, py_loader_impl_capi_to_value_type(impl, obj));

	if (v == NULL)
	{
		PyErr_SetString(PyExc_ValueError, "Failed to convert the Python object to MetaCall value.");
		goto error_none;
	}

	capsule = PyCapsule_New(v, py_loader_capsule_reference_id, &py_loader_port_value_reference_destroy);

	if (capsule == NULL)
	{
		goto error_value;
	}

	return capsule;

error_value:
	metacall_value_destroy(v);
error_none:
	return py_loader_port_none();
}

static PyObject *py_loader_port_value_dereference(PyObject *self, PyObject *args)
{
	static const char format[] = "O:metacall_value_dereference";
	PyObject *capsule;
	const char *name = NULL;
	void *v;
	loader_impl impl;
	PyObject *result;

	(void)self;

	/* Parse arguments */
	if (!PyArg_ParseTuple(args, (char *)format, &capsule))
	{
		PyErr_SetString(PyExc_TypeError, "Invalid number of arguments, use it like: metacall_value_dereference(ptr);");
		return py_loader_port_none();
	}

	/* Check if it is a valid reference */
	if (!PyCapsule_CheckExact(capsule))
	{
		PyErr_SetString(PyExc_TypeError, "Invalid parameter type in first argument must be a PyCapsule (i.e a previously allocated pointer)");
		return py_loader_port_none();
	}

	/* Check if it is a valid MetaCall reference */
	name = PyCapsule_GetName(capsule);

	if (name != py_loader_capsule_reference_id)
	{
		PyErr_SetString(PyExc_TypeError, "Invalid reference, argument must be a PyCapsule from MetaCall");
		return py_loader_port_none();
	}

	/* Get the value */
	v = PyCapsule_GetPointer(capsule, name);

	if (v == NULL)
	{
		return py_loader_port_none();
	}

	/* Obtain Python loader implementation */
	impl = loader_get_impl(py_loader_tag);

	/* TODO: When using the port outside MetaCall this is going to segfault for functions and similar
	* structures that require py loader internal structure to be initialized. For those cases, we
	* must implement this: https://github.com/metacall/core/issues/231
	*/
	result = py_loader_impl_value_to_capi(impl, value_type_id(v), v);

	if (result == NULL)
	{
		PyErr_SetString(PyExc_ValueError, "Failed to convert the MetaCall value to Python object.");
		return py_loader_port_none();
	}

	return result;
}

static PyMethodDef metacall_methods[] = {
	{ "metacall_load_from_file", py_loader_port_load_from_file, METH_VARARGS,
		"Loads a script from file." },
	{ "metacall_load_from_file_export", py_loader_port_load_from_file_export, METH_VARARGS,
		"Loads a script from file (returns a local handle instead of loading it in the global namespace)." },
	{ "metacall_load_from_package", py_loader_port_load_from_package, METH_VARARGS,
		"Loads a script from a package." },
	{ "metacall_load_from_package_export", py_loader_port_load_from_package_export, METH_VARARGS,
		"Loads a script from package (returns a local handle instead of loading it in the global namespace)." },
	{ "metacall_load_from_memory", py_loader_port_load_from_memory, METH_VARARGS,
		"Loads a script from a string." },
	{ "metacall_inspect", py_loader_port_inspect, METH_NOARGS,
		"Get information about all loaded objects." },
	{ "metacall", py_loader_port_invoke, METH_VARARGS,
		"Call a function anonymously." },
	{ "metacall_value_create_ptr", py_loader_port_value_create_ptr, METH_VARARGS,
		"Create a new value of type Pointer." },
	{ "metacall_value_reference", py_loader_port_value_reference, METH_VARARGS,
		"Create a new value of type Pointer." },
	{ "metacall_value_dereference", py_loader_port_value_dereference, METH_VARARGS,
		"Get the data which a value of type Pointer is pointing to." },
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

PyMODINIT_FUNC PY_LOADER_PORT_NAME_FUNC(void)
{
	static PyObject *module = NULL;

	if (module == NULL)
	{
		module = PyModule_Create(&metacall_definition);
	}

	return module;
}
