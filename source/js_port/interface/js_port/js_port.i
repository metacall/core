/*
 *	MetaCall SWIG Wrapper by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A complete infraestructure for supporting multiple langauge bindings in MetaCall.
 *
 */

#ifndef METACALL_SWIG_WRAPPER_JS_PORT_I
#define METACALL_SWIG_WRAPPER_JS_PORT_I 1

/* -- Headers -- */

#ifdef SWIG

	%module js_port

	%{
		#include <js_port/js_port.h>

		#include <metacall/metacall_api.h>
		#include <metacall/metacall.h>
		#include <reflect/reflect_value_type.h>

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

	#ifdef METACALL_API
	#	undef METACALL_API
	#	define METACALL_API
	#endif

	%include <metacall/metacall.h>


	%{
		#include <dynlink/dynlink.h>

		#if (NODE_MODULE_VERSION < 0x000C)
			extern "C" void js_port_initialize(v8::Handle<v8::Object> exports);
		#else
			extern "C" void js_port_initialize(v8::Handle<v8::Object> exports, v8::Handle<v8::Object> module);
		#endif

		extern "C" DYNLINK_SYMBOL_EXPORT(js_port_initialize);
	%}

#endif /* SWIG */

#endif /* METACALL_SWIG_WRAPPER_JS_PORT_I */
