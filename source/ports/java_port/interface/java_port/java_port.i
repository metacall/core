/*
 *	MetaCall SWIG Wrapper by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef METACALL_SWIG_WRAPPER_JAVA_PORT_I
#define METACALL_SWIG_WRAPPER_JAVA_PORT_I 1

/* -- Headers -- */

#if defined(SWIG) && defined(SWIGJAVA)

	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
		%module java_portd
	#else
		%module java_port
	#endif

	%{
		#include <java_port/java_port.h>

		#include <metacall/metacall_api.h>
		#include <metacall/metacall.h>
		#include <metacall/metacall_value.h>

		#include <jni.h>
	%}

	%include <java_port/java_port.h>

	/* TODO: Remove this once java loader is implemented */
	%init
	%{
		metacall_initialize();
	%}

	%import <java_port/java_port_impl.i>

	%include <metacall/metacall_api.h>

	#ifdef METACALL_API
	#	undef METACALL_API
	#	define METACALL_API
	#endif

	%include <metacall/metacall.h>

#endif /* SWIG && SWIGJAVA */

#endif /* METACALL_SWIG_WRAPPER_JAVA_PORT_I */
