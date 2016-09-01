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
		$1 = value_create_long(PyLong_AsLong($input));
	}
#endif

#endif /* METACALL_SWIG_WRAPPER_PY_PORT_TYPEMAP_I */
