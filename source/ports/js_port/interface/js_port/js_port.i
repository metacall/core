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

#ifndef METACALL_SWIG_WRAPPER_JS_PORT_I
#define METACALL_SWIG_WRAPPER_JS_PORT_I 1

/* -- Headers -- */

#if defined(SWIG) && defined(SWIGJAVASCRIPT) && defined(SWIG_JAVASCRIPT_V8)

	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
		%module js_portd
		%{
			#define JS_PORT_INITIALIZE_NAME js_portd_initialize
		%}
	#else
		%module js_port
		%{
			#define JS_PORT_INITIALIZE_NAME js_port_initialize
		%}
	#endif

	%{
		#include <js_port/js_port.h>

		#include <metacall/metacall_api.h>
		#include <metacall/metacall.h>
		#include <metacall/metacall_value.h>

		#include <libplatform/libplatform.h>
		#include <v8.h>

		#ifdef ENABLE_DEBUGGER_SUPPORT
		#	include <v8-debug.h>
		#endif /* ENABLE_DEBUGGER_SUPPORT */

		using namespace v8;
	%}

	%include <js_port/js_port.h>

	%init
	%{
		metacall_initialize();
	%}

	%import <js_port/js_port_impl.i>

	%include <metacall/metacall_api.h>

	%{
		#if (NODE_MODULE_VERSION < 0x000C)
			extern "C" METACALL_API void JS_PORT_INITIALIZE_NAME(v8::Handle<v8::Object> exports);
		#else
			extern "C" METACALL_API void JS_PORT_INITIALIZE_NAME(v8::Handle<v8::Object> exports, v8::Handle<v8::Object> module);
		#endif
	%}

	#ifdef METACALL_API
	#	undef METACALL_API
	#	define METACALL_API
	#endif

	%include <metacall/metacall.h>

#endif /* SWIG && SWIGJAVASCRIPT && SWIG_JAVASCRIPT_V8 */

#endif /* METACALL_SWIG_WRAPPER_JS_PORT_I */
