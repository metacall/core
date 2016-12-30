/*
 *	MetaCall SWIG Wrapper by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A complete infraestructure for supporting multiple langauge bindings in MetaCall.
 *
 */

#ifndef METACALL_SWIG_WRAPPER_RB_PORT_I
#define METACALL_SWIG_WRAPPER_RB_PORT_I 1

/* -- Headers -- */

#ifdef SWIG

	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
		%module rb_portd
	#else
		%module rb_port
	#endif

	%{
		#include <rb_port/rb_port.h>

		#include <metacall/metacall_api.h>
		#include <metacall/metacall.h>
		#include <reflect/reflect_value_type.h>

		#include <ruby.h>
		#include <ruby/intern.h>
	%}

	%include <rb_port/rb_port.h>

	#ifndef LOADER_LAZY
		%init
		%{
			metacall_initialize();
		%}
	#endif

	%import <rb_port/rb_port_impl.i>

	%include <metacall/metacall_api.h>

	#ifdef METACALL_API
	#	undef METACALL_API
	#	define METACALL_API
	#endif

	%include <metacall/metacall.h>

#endif

#endif /* METACALL_SWIG_WRAPPER_RB_PORT_I */
