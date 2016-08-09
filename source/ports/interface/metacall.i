/*
 *	MetaCall SWIG Wrapper by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A complete infraestructure for supporting multiple langauge bindings in MetaCall.
 *
 */

#ifndef METACALL_SWIG_WRAPPER_I
#define METACALL_SWIG_WRAPPER_I 1

#ifdef SWIG
	%module metacall
	%{
		#include <metacall/metacall.h>
	%}

	%include <metacall/metacall.h>
#endif

#endif /* METACALL_SWIG_WRAPPER_I */
