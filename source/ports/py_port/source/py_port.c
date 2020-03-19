/*
 *	MetaCall SWIG Wrapper by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
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

#include <py_port/py_port.h>

#include <metacall/metacall.h>

PyObject * metacall_value_to_python(void * value)
{
	switch (metacall_value_id(value))
	{
		case METACALL_BOOL :
		{
			return PyBool_FromLong((long)metacall_value_to_bool(value));
		}

		case METACALL_CHAR :
		{
			/*return PyInt_FromLong((long)metacall_value_to_char(value));*/
			return PyLong_FromLong((long)metacall_value_to_char(value));
		}

		case METACALL_SHORT :
		{
			/*return PyInt_FromLong((long)metacall_value_to_short(value));*/
			return PyLong_FromLong((long)metacall_value_to_short(value));
		}

		case METACALL_INT :
		{
			/*return PyInt_FromLong((long)metacall_value_to_int(value));*/
			return PyLong_FromLong((long)metacall_value_to_int(value));
		}

		case METACALL_LONG :
		{
			return PyLong_FromLong(metacall_value_to_long(value));
		}

		case METACALL_FLOAT :
		{
			return PyFloat_FromDouble((double)metacall_value_to_float(value));
		}

		case METACALL_DOUBLE :
		{
			return PyFloat_FromDouble(metacall_value_to_double(value));
		}

		case METACALL_STRING :
		{
			return PyUnicode_FromString(metacall_value_to_string(value));
		}

		case METACALL_ARRAY :
		{
			void ** array = metacall_value_to_array(value);
			size_t count = metacall_value_count(value);
			Py_ssize_t iterator;
			PyObject * list = PyList_New((Py_ssize_t)count);

			/* Set up all elements */
			for (iterator = 0; iterator < (Py_ssize_t)count; ++iterator)
			{
				/* TODO: Review recursion overflow */
				PyObject * item = metacall_value_to_python(array[iterator]);

				PyList_SetItem(list, iterator, item);
			}

			return list;
		}

		case METACALL_MAP :
		{
			/* TODO */
			return Py_None;
		}

		case METACALL_PTR :
		{
			#if PY_MAJOR_VERSION == 2
				/* TODO */
			#elif PY_MAJOR_VERSION == 3
				return PyCapsule_New(metacall_value_to_ptr(value), NULL, NULL);
			#endif
		}

		case METACALL_NULL :
		{
			return Py_None;
		}

		default :
		{
			PyErr_SetString(PyExc_ValueError, "Unsupported return type");

			return Py_None;
		}
	}
}
