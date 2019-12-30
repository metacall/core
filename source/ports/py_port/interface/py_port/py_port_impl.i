/*
 *	MetaCall SWIG Wrapper by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef METACALL_SWIG_WRAPPER_PY_PORT_IMPL_I
#define METACALL_SWIG_WRAPPER_PY_PORT_IMPL_I 1

#ifdef __cplusplus
extern "C" {
#endif

/* -- Ignores -- */

%ignore metacall_null_args;

%ignore metacallv;

%ignore metacallvf;

%ignore metacall_serial; /* TODO */

%ignore metacall_register; /* TODO */

%ignore metacall_load_from_package; /* TODO */

/* -- Type Maps -- */

/**
*  @brief
*    Transform load mechanism from Python string into
*    a valid load from memory format (buffer and size)
*/
%typemap(in) (const char * buffer, size_t size, void ** handle)
{
	char * buffer_str = NULL;

	Py_ssize_t length = 0;

	%#if PY_MAJOR_VERSION == 2
		if (PyString_AsStringAndSize($input, &buffer_str, &length) == -1)
		{
			PyErr_SetString(PyExc_TypeError, "Invalid string conversion");

			SWIG_fail;
		}
	%#elif PY_MAJOR_VERSION == 3
		buffer_str = (char *)PyUnicode_AsUTF8AndSize($input, &length);

		if (buffer_str == NULL)
		{
			PyErr_SetString(PyExc_TypeError, "Invalid string conversion");

			SWIG_fail;
		}
	%#endif

	$1 = buffer_str;

	$2 = (length + 1);
}

/**
*  @brief
*    Transform load mechanism from Python list into
*    a valid load from file format (array of strings)
*/
%typemap(in) (const char * paths[], size_t size, void ** handle)
{
	if (PyList_Check($input))
	{
		size_t iterator, size = PyList_Size($input);

		if (size == 0)
		{
			PyErr_SetString(PyExc_ValueError, "Empty script path list");

			return Py_None;
		}

		$1 = (char **)malloc(sizeof(char *) * size);

		if ($1 == NULL)
		{
			PyErr_SetString(PyExc_ValueError, "Invalid argument allocation");

			SWIG_fail;
		}

		$2 = size;

		for (iterator = 0; iterator < size; ++iterator)
		{
			PyObject * object_str = PyList_GetItem($input, iterator);

			int check_str =
				%#if PY_MAJOR_VERSION == 2
					PyString_Check(object_str);
				%#elif PY_MAJOR_VERSION == 3
					PyUnicode_Check(object_str);
				%#endif

			if (check_str != 0)
			{
				char * str = NULL;

				Py_ssize_t length = 0;

				%#if PY_MAJOR_VERSION == 2
					if (PyString_AsStringAndSize(object_str, &str, &length) == -1)
					{
						size_t alloc_iterator;

						for (alloc_iterator = 0; alloc_iterator < iterator; ++alloc_iterator)
						{
							free($1[alloc_iterator]);
						}

						PyErr_SetString(PyExc_TypeError, "Invalid string conversion");

						SWIG_fail;
					}
				%#elif PY_MAJOR_VERSION == 3
					str = (char *)PyUnicode_AsUTF8AndSize(object_str, &length);

					if (str == NULL)
					{
						size_t alloc_iterator;

						for (alloc_iterator = 0; alloc_iterator < iterator; ++alloc_iterator)
						{
							free($1[alloc_iterator]);
						}

						PyErr_SetString(PyExc_TypeError, "Invalid string conversion");

						SWIG_fail;
					}
				%#endif

				$1[iterator] = (char *)malloc(sizeof(char) * (length + 1));

				if ($1[iterator] == NULL)
				{
					size_t alloc_iterator;

					for (alloc_iterator = 0; alloc_iterator < iterator; ++alloc_iterator)
					{
						free($1[alloc_iterator]);
					}

					free($1);

					PyErr_SetString(PyExc_ValueError, "Invalid string path allocation");

					SWIG_fail;
				}

				memcpy($1[iterator], str, length);

				$1[iterator][length] = '\0';
			}
		}
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "Invalid parameter type (a list must be used)");

		SWIG_fail;
	}
}

/**
*  @brief
*    Transform variadic arguments from Python into
*    a valid metacallv format with values
*/
%typemap(in) (const char * name, ...)
{
	void ** args;
	size_t args_size, args_count;

	/* Format string */
	$1 = (char *)PyUnicode_AsUTF8($input);

	/* Variable length arguments */
	args_size = PyTuple_Size(varargs);

	if (args_size > 0)
	{
		/* TODO: Remove this by a local array? */
		args = (void **) malloc(args_size * sizeof(void *));

		if (args == NULL)
		{
			PyErr_SetString(PyExc_ValueError, "Invalid argument allocation");

			SWIG_fail;
		}

		for (args_count = 0; args_count < args_size; ++args_count)
		{
			PyObject * py_arg = PyTuple_GetItem(varargs, args_count);

			if (PyBool_Check(py_arg))
			{
				boolean b = (PyObject_IsTrue(py_arg) == 1) ? 1L : 0L;

 				args[args_count] = metacall_value_create_bool(b);
			}
			/*if (PyInt_Check(py_arg))
			{
				args[args_count] = metacall_value_create_int((int) PyInt_AsLong(py_arg));
			}
			*/else if (PyLong_Check(py_arg))
			{
				args[args_count] = metacall_value_create_long(PyLong_AsLong(py_arg));
			}
			else if (PyFloat_Check(py_arg))
			{
				args[args_count] = metacall_value_create_double(PyFloat_AsDouble(py_arg));
			}
			else if (PyUnicode_Check(py_arg))
			{
				Py_ssize_t size;

				const char * str = PyUnicode_AsUTF8AndSize(py_arg, &size);

				args[args_count] = metacall_value_create_string(str, (size_t)size);
			}
			else if (PyList_Check(py_arg))
			{
				Py_ssize_t iterator, length = 0;
				void ** array_value;

				length = PyList_Size(py_arg);

				args[args_count] = metacall_value_create_array(NULL, (size_t)length);

				array_value = metacall_value_to_array(args[args_count]);

				for (iterator = 0; iterator < length; ++iterator)
				{
					PyObject * element = PyList_GetItem(py_arg, iterator);

					/* TODO */
					(void)element;

					/* TODO: Review recursion overflow */
					/*
					array_value[iterator] = py_loader_impl_return(element, py_loader_impl_get_return_type(element));
					*/
				}
			}
			else if (PyCapsule_CheckExact(py_arg))
			{
				void * ptr = NULL;

				%#if PY_MAJOR_VERSION == 2
					/* TODO */
				%#elif PY_MAJOR_VERSION == 3
					ptr = PyCapsule_GetPointer(py_arg, NULL);

					args[args_count] = metacall_value_create_ptr(ptr);
				%#endif
			}
			else
			{
				/* TODO: Remove this by a local array? */
				free(args);

				PyErr_SetString(PyExc_ValueError, "Unsupported argument type");

				SWIG_fail;
			}
		}

		$2 = (void *) args;
	}
	else
	{
		$2 = (void *) NULL;
	}
}

/* -- Features -- */

/**
*  @brief
*    Execute the load from memory
*
*  @return
*    Zero if success, different from zero otherwise
*/
%feature("action") metacall_load_from_memory
{
	const char * tag = (const char *)arg1;

	char * buffer = (char *)arg2;

	size_t size = (size_t)arg3;

	result = metacall_load_from_memory(tag, (const char *)buffer, size, NULL);
}

/**
*  @brief
*    Execute the load from file
*
*  @return
*    Zero if success, different from zero otherwise
*/
%feature("action") metacall_load_from_file
{
	const char * tag = (const char *)arg1;

	char ** paths = (char **)arg2;

	size_t iterator, size = arg3;

	result = metacall_load_from_file(tag, (const char **)paths, size, NULL);

	for (iterator = 0; iterator < size; ++iterator)
	{
		free(paths[iterator]);
	}

	free(paths);
}

/**
*  @brief
*    Execute the call and transform return
*    value into a valid Python format
*
*  @return
*    A value converted into Python format
*/
%feature("action") metacall
{
	size_t args_count, args_size;
	void ** args;
	void * ret;

	args_size = PyTuple_Size(varargs);
	args = (void **) arg2;

	if (args != NULL)
	{
		/* Execute call */
		ret = metacallv(arg1, args);

		/* Clear args */
		for (args_count = 0; args_count < args_size; ++args_count)
		{
			metacall_value_destroy(args[args_count]);
		}

		/* TODO: Remove this by a local array? */
		free(args);
	}
	else
	{
		void * metacall_null_args_proxy[1] =
		{
			NULL
		};

		ret = metacallv(arg1, metacall_null_args_proxy);
	}

	/* Return value */
	if (ret != NULL)
	{
		$result = metacall_value_to_python(ret);

		if ($result == NULL)
		{
			$result = Py_None;
		}

		metacall_value_destroy(ret);
	}
	else
	{
		$result = Py_None;
	}

	return $result;
}

#ifdef __cplusplus
}
#endif

#endif /* METACALL_SWIG_WRAPPER_PY_PORT_IMPL_I */
