/*
 *	MetaCall SWIG Wrapper by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A complete infraestructure for supporting multiple langauge bindings in MetaCall.
 *
 */

#ifndef METACALL_SWIG_WRAPPER_PY_PORT_I
#define METACALL_SWIG_WRAPPER_PY_PORT_I 1

/* -- Headers -- */

#ifdef SWIG
	%module metacall_py_port

	%{
		#include <py_port/py_port.h>

		#include <metacall/metacall.h>
		#include <reflect/reflect_value_type.h>

		#include <format/format.h> /* TODO: Remove this */

		#include <Python.h>
	%}

	/*%include <py_port/py_port_typemap.i>*/

	%include <py_port/py_port.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

/*
#include <stdlib.h>
#include <stdarg.h>
*/

/* -- Type Definitions -- */

typedef void * value;

/* -- Global Variables -- */

extern void * metacall_null_args[1];

/* -- Methods -- */

/**
*  @brief
*    Initialize MetaCall library singleton
*
*  @return
*    Zero if success, different from zero otherwise
*/
int metacall_initialize(void);

/**
*  @brief
*    Amount of function call arguments supported by MetaCall
*
*  @return
*    Number of arguments suported
*/
size_t metacall_args_size(void);

/**
*  @brief
*    Loads a script from file specified by @path
*
*  @param[in] path
*    Path of the file
*
*  @return
*    Zero if success, different from zero otherwise
*/
int metacall_load_from_file(const char * path);

/**
*  @brief
*    Loads a script from memory
*
*  @param[in] extension
*    Extension of the script
*
*  @param[in] buffer
*    Memory block representing the string of the script
*
*  @param[in] size
*    Memory block representing the string of the script
*
*  @return
*    Zero if success, different from zero otherwise
*/
int metacall_load_from_memory(const char * extension, const char * buffer, size_t size);

/**
*  @brief
*    Call a function anonymously by value array @args
*
*  @param[in] name
*    Name of the function
*
*  @param[in] args
*    Array of pointers to data
*
*  @return
*    Pointer to value containing the result of the call
*/
value metacallv(const char * name, void * args[]);

/**
*  @brief
*    Call a function anonymously by variable arguments @va_args
*
*  @param[in] name
*    Name of the function
*
*  @param[in] va_args
*    Varidic function parameters
*
*  @return
*    Pointer to value containing the result of the call
*/
value metacall(const char * name, ...);

/**
*  @brief
*    Destroy MetaCall library singleton
*
*  @return
*    Zero if success, different from zero otherwise
*/
int metacall_destroy(void);

/**
*  @brief
*    Provide the module information
*
*  @return
*    Static string containing module information
*/
const char * metacall_print_info(void);

/* -- Typemaps -- */

/**
*  @brief
*    Transform variadic arguments from Python into
*    a valid metacallv format with values
*/
%typemap(in) (const char * name, ...)
{
	value * argv;
	int    argc;
	int    i;

	/* Format string */
	$1 = PyUnicode_AsUTF8($input);

	printf("\nA) ");

	PyObject_Print($input, stdout, 0);

	printf("\n");

	/* Variable length arguments */
	argc = PyTuple_Size(varargs);

	printf("B) ");

	PyObject_Print(varargs, stdout, 0);

	printf("\n");

	argv = (value *) malloc(argc * sizeof(value));

	printf("C) Number of arguments: %d\n", argc);

	for (i = 0; i < argc; ++i)
	{
		PyObject * o = PyTuple_GetItem(varargs, i);

		printf("D) ");

		PyObject_Print(o, stdout, 0);

		printf("\n");

		/*if (PyInt_Check(o))
		{
			argv[i] = value_create_int((int) PyInt_AsLong(o));
		}
		else */if (PyLong_Check(o))
		{
			argv[i] = value_create_long(PyLong_AsLong(o));
		}
		else if (PyFloat_Check(o))
		{
			argv[i] = value_create_double(PyFloat_AsDouble(o));
		}
		else if (PyUnicode_Check(o))
		{
			Py_ssize_t size;

			const char * str = PyUnicode_AsUTF8AndSize(o, &size);

			argv[i] = value_create_string(str, (size_t)size);
		}
		else
		{
			PyErr_SetString(PyExc_ValueError,"Unsupported argument type");
			free(argv);
			return NULL;
		}
	}
	$2 = (void *) argv;
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
	int			i, vc;
	value		*args, ret;

	vc = PyTuple_Size(varargs);
	args   = (value *) arg2;

	/* Execute call */
	ret = metacallv(arg1, args);

	printf("E) ret_value: %p\n", ret);

	/* Clear args */
	for (i = 0; i < vc; ++i)
	{
		value_destroy(args[i]);
	}

	free(args);

	/* Return value */
	if (ret != NULL)
	{
		switch (value_type_id(ret))
		{
			case TYPE_INT :
			{
				$result = PyInt_FromLong((long)value_to_int(ret));
				break;
			}

			case TYPE_LONG :
			{
				$result = PyLong_FromLong(value_to_long(ret));
				break;
			}

			case TYPE_DOUBLE :
			{
				$result = PyFloat_FromDouble(value_to_double(ret));
				break;
			}

			case TYPE_STRING :
			{
				$result = PyUnicode_FromString(value_to_string(ret));
				break;
			}

			default :
			{
				PyErr_SetString(PyExc_ValueError, "Unsupported return type");
				$result = Py_None;
			}
		}

		value_destroy(ret);
	}
	else
	{
		$result = Py_None;
	}

	printf("F) Result: ");

	PyObject_Print($result, stdout, 0);

	printf("\n");
}

#ifdef __cplusplus
}
#endif

#endif /* METACALL_SWIG_WRAPPER_PY_PORT_I */
