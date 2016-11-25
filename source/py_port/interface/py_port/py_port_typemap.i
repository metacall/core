/*
 *	MetaCall SWIG Wrapper by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A complete infraestructure for supporting multiple langauge bindings in MetaCall.
 *
 */

#ifndef METACALL_SWIG_WRAPPER_PY_PORT_TYPEMAP_I
#define METACALL_SWIG_WRAPPER_PY_PORT_TYPEMAP_I 1

#ifdef SWIG
	%module metacall_py_port_typemap

	/* Convert from Python to C */
	%typemap(in) int
	{
		#if defined(PY_MAJOR_VERSION) && (PY_MAJOR_VERSION == 2)
			$1 = PyInt_AsLong($input);
		#elif defined(PY_MAJOR_VERSION) && (PY_MAJOR_VERSION == 3)
			$1 = PyLong_AsLong($input);
		#else
			$1 = -1
		#endif
	}

	/* Convert from C to Python */
	%typemap(out) int
	{
		#if defined(PY_MAJOR_VERSION) && (PY_MAJOR_VERSION == 2)
			$result = PyInt_FromLong($1);
		#elif defined(PY_MAJOR_VERSION) && (PY_MAJOR_VERSION == 3)
			$result = PyLong_FromLong($1);
		#else
			$result = -1
		#endif
	}
#endif

#endif /* METACALL_SWIG_WRAPPER_PY_PORT_TYPEMAP_I */
