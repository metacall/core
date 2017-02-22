/*
 *	MetaCall SWIG Wrapper by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A complete infraestructure for supporting multiple langauge bindings in MetaCall.
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

	#ifndef LOADER_LAZY
		%init
		%{
			metacall_initialize();
		%}
	#endif

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
