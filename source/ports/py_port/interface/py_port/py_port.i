/*
 *	MetaCall SWIG Wrapper by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A complete infraestructure for supporting multiple langauge bindings in MetaCall.
 *
 */

#ifndef METACALL_SWIG_WRAPPER_PY_PORT_I
#define METACALL_SWIG_WRAPPER_PY_PORT_I 1

#ifdef SWIG
	%module py_port
	%{
		#include <py_port/py_port.h>
	%}

	/*%include <py_port/py_port_typemap.i>*/

	%include <py_port/py_port.h>
#endif

#endif /* METACALL_SWIG_WRAPPER_PY_PORT_I */
