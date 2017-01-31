/*
 *	MetaCall SWIG Wrapper by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A complete infraestructure for supporting multiple langauge bindings in MetaCall.
 *
 */

#ifndef METACALL_SWIG_WRAPPER_PY_PORT_IMPL_I
#define METACALL_SWIG_WRAPPER_PY_PORT_IMPL_I 1

#ifdef __cplusplus
extern "C" {
#endif

/* -- Type Maps -- */

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
	$1 = PyUnicode_AsUTF8($input);

	/* Variable length arguments */
	args_size = PyTuple_Size(varargs);

	if (args_size == 0)
	{
		PyErr_SetString(PyExc_ValueError, "Invalid number of arguments");

		return Py_None;
	}

	/* TODO: Remove this by a local array? */
	args = (void **) malloc(args_size * sizeof(void *));

	if (args == NULL)
	{
		PyErr_SetString(PyExc_ValueError, "Invalid argument allocation");

		SWIG_fail;

		return NULL;
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
		else
		{
			/* TODO: Remove this by a local array? */
			free(args);

			PyErr_SetString(PyExc_ValueError, "Unsupported argument type");

			SWIG_fail;

			return NULL;
		}
	}

	$2 = (void *) args;
}

/* -- Features -- */

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

	/* Execute call */
	ret = metacallv(arg1, args);

	/* Clear args */
	for (args_count = 0; args_count < args_size; ++args_count)
	{
		metacall_value_destroy(args[args_count]);
	}

	/* TODO: Remove this by a local array? */
	free(args);

	/* Return value */
	if (ret != NULL)
	{
		switch (metacall_value_id(ret))
		{

			case METACALL_BOOL :
			{
				$result = PyBool_FromLong((long)metacall_value_to_bool(ret));

				break;
			}

			case METACALL_CHAR :
			{
				/*$result = PyInt_FromLong((long)metacall_value_to_char(ret));*/
				$result = PyLong_FromLong((long)metacall_value_to_char(ret));

				break;
			}

			case METACALL_SHORT :
			{
				/*$result = PyInt_FromLong((long)metacall_value_to_short(ret));*/
				$result = PyLong_FromLong((long)metacall_value_to_short(ret));

				break;
			}

			case METACALL_INT :
			{
				/*$result = PyInt_FromLong((long)metacall_value_to_int(ret));*/
				$result = PyLong_FromLong((long)metacall_value_to_int(ret));

				break;
			}

			case METACALL_LONG :
			{
				$result = PyLong_FromLong(metacall_value_to_long(ret));

				break;
			}

			case METACALL_FLOAT :
			{
				$result = PyFloat_FromDouble((double)metacall_value_to_float(ret));

				break;
			}

			case METACALL_DOUBLE :
			{
				$result = PyFloat_FromDouble(metacall_value_to_double(ret));

				break;
			}

			case METACALL_STRING :
			{
				$result = PyUnicode_FromString(metacall_value_to_string(ret));

				break;
			}

			default :
			{
				PyErr_SetString(PyExc_ValueError, "Unsupported return type");

				$result = Py_None;
			}
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
