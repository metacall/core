/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading javascript code at run-time into a process.
 *
 */

#include <js_loader/js_loader_impl.h>

#include <loader/loader_impl.h>

#include <reflect/type.h>
#include <reflect/function.h>
#include <reflect/scope.h>
#include <reflect/context.h>

#include <adt/hash_map.h>
#include <adt/hash_map_str.h>

#include <cstdlib>
#include <cstring>

#include <new>
#include <iostream>

#include <libplatform/libplatform.h>
#include <v8.h> /* version: 5.3.0 */

#ifdef ENABLE_DEBUGGER_SUPPORT
#	include <v8-debug.h>
#endif /* ENALBLE_DEBUGGER_SUPPORT */

using namespace v8;

class ArrayBufferAllocator : public ArrayBuffer::Allocator
{
	public:
		virtual void * Allocate(size_t length)
		{
			void * data = AllocateUninitialized(length);

			return (data == NULL) ? data : memset(data, 0, length);
		}

		virtual void * AllocateUninitialized(size_t length)
		{
			return malloc(length);
		}

		virtual void Free(void * data, size_t)
		{
			free(data);
		}
};

typedef struct loader_impl_js_type
{
	Platform * platform;

} * loader_impl_js;

typedef struct loader_impl_js_handle_type
{
	Isolate * isolate;

} * loader_impl_js_handle;

int function_js_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

void function_js_interface_invoke(function func, function_impl impl, function_args args)
{
	(void)func;
	(void)impl;
	(void)args;
}

void function_js_interface_destroy(function func, function_impl impl)
{
	(void)func;
	(void)impl;
}

function_interface function_js_singleton()
{
	static struct function_interface_type js_interface =
	{
		&function_js_interface_create,
		&function_js_interface_invoke,
		&function_js_interface_destroy
	};

	return &js_interface;
}

loader_impl_data js_loader_impl_initialize(loader_impl impl)
{
	loader_impl_js js_impl = new loader_impl_js_type();

	(void)impl;

	if (js_impl != nullptr)
	{
		/* const char * icu_path = "."; */

		const char * external_startup_data = "c_loader";

		/* V8::InitializeICUDefaultLocation(icu_path); */

		V8::InitializeExternalStartupData(external_startup_data);

		js_impl->platform = platform::CreateDefaultPlatform();

		V8::InitializePlatform(js_impl->platform);

		V8::Initialize();

		return static_cast<loader_impl_data>(js_impl);
	}

	return NULL;
}

int js_loader_impl_execution_path(loader_impl impl, loader_naming_path path)
{
	(void)impl;
	(void)path;

	return 0;
}

loader_handle js_loader_impl_load(loader_impl impl, loader_naming_path path, loader_naming_name name)
{
	loader_impl_js_handle js_handle = new loader_impl_js_handle_type();

	(void)impl;
	(void)path;
	(void)name;

	if (js_handle != nullptr)
	{
		ArrayBufferAllocator allocator;

		Isolate::CreateParams create_params;

		create_params.array_buffer_allocator = &allocator;

		js_handle->isolate = Isolate::New(create_params);

		if (js_handle->isolate != nullptr)
		{
			HandleScope handle_scope(js_handle->isolate);

			Local<Context> ctx = Context::New(js_handle->isolate);

			Context::Scope ctx_scope(ctx);

			Local<String> source = String::NewFromUtf8(js_handle->isolate,
				"'Hello' + ', World!'", NewStringType::kNormal).ToLocalChecked();

			Local<Script> script = Script::Compile(ctx, source).ToLocalChecked();

			Local<Value> result = script->Run(ctx).ToLocalChecked();

			String::Utf8Value utf8(result);

			std::cout << "Script result: " << *utf8 << std::endl;

			return js_handle;
		}

/*		HandleScope handle_scope;

		Handle<String> script_source(String::New("print('hello world');"));
		Handle<String> script_name(String::New("test"));

		Handle<ObjectTemplate> global = ObjectTemplate::New();
		Handle<Context> ctx = Context::New(NULL, global);
		Context::Scope ctx_scope(ctx);

		Handle<Script> script;

		{
			TryCatch try_catch_compile;

			script = Script::Compile(script_source, script_name);

			if (!script.IsEmpty() && !try_catch_compile.HasCaught())
			{
				TryCatch try_catch_run;

				script->Run();

				if (try_catch_run.HasCaught())
				{
					return static_cast<loader_handle>(js_handle);
				}
*/
				/* js_throw_exception(try_catch_run); */
/*			}

			if (try_catch_compile.HasCaught())
			{

*/				/* js_throw_exception(try_catch_compile); */
/*			}
		}


		delete js_handle;
*/	}

	return NULL;
}

int js_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_js_handle js_handle = static_cast<loader_impl_js_handle>(handle);

	(void)impl;

	if (js_handle != nullptr)
	{
		if (js_handle->isolate != nullptr)
		{
			js_handle->isolate->Dispose();
		}

		delete js_handle;

		return 0;
	}

	return 1;
}

int js_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	/* loader_impl_js_handle js_handle = (loader_impl_js_handle)handle; */

	(void)impl;
	(void)handle;
	(void)ctx;

	return 0;
}

int js_loader_impl_destroy(loader_impl impl)
{
	loader_impl_js js_impl = static_cast<loader_impl_js>(loader_impl_get(impl));

	if (js_impl != nullptr)
	{
		V8::Dispose();

		V8::ShutdownPlatform();

		if (js_impl->platform != nullptr)
		{
			delete js_impl->platform;

			js_impl->platform = nullptr;
		}

		delete js_impl;

		return 0;
	}

	return 1;
}

