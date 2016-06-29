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
#include <string>
#include <fstream>
#include <streambuf>

#include <libplatform/libplatform.h>
#include <v8.h> /* version: 5.1.117 */

#ifdef ENABLE_DEBUGGER_SUPPORT
#	include <v8-debug.h>
#endif /* ENALBLE_DEBUGGER_SUPPORT */

using namespace v8;

MaybeLocal<String> js_loader_impl_read_script(Isolate * isolate, loader_naming_path path);

class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator
{
	public:
		virtual void * Allocate(size_t length)
		{
			void * data = AllocateUninitialized(length);

			if (data != NULL)
			{
				return memset(data, 0, length);
			}

			return NULL;
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
	Isolate * isolate;
	Isolate::CreateParams isolate_create_params;
	Isolate::Scope * isolate_scope;
	ArrayBufferAllocator allocator;

} * loader_impl_js;

typedef class loader_impl_js_handle_type
{
	public:
		loader_impl_js_handle_type(loader_impl_js js_impl,
			loader_naming_path path/*, loader_naming_name name*/) :
				handle_scope(js_impl->isolate),
				ctx_impl(Context::New(js_impl->isolate)), ctx_scope(ctx_impl)
		{

			Local<String> source = js_loader_impl_read_script(js_impl->isolate, path).ToLocalChecked();

			script = Script::Compile(ctx_impl, source).ToLocalChecked();

			Local<Value> result = script->Run(ctx_impl).ToLocalChecked();

			String::Utf8Value utf8(result);

			std::cout << "Result: " << *utf8 << std::endl;
		}

		int discover(loader_impl_js js_impl, context ctx)
		{
			(void)js_impl;
			(void)ctx;

			return 0;
		}

		~loader_impl_js_handle_type()
		{

		}

	private:
		HandleScope handle_scope;
		Local<Context> ctx_impl;
		Context::Scope ctx_scope;
		Local<Script> script;

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

void js_loader_impl_read_file(loader_naming_path path, std::string & source)
{
	std::ifstream file(path);

	file.seekg(0, std::ios::end);

	source.reserve(file.tellg());

	file.seekg(0, std::ios::beg);

	source.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

MaybeLocal<String> js_loader_impl_read_script(Isolate * isolate, loader_naming_path path)
{
	MaybeLocal<String> result;

	std::string source;

	js_loader_impl_read_file(path, source);

	if (!source.empty())
	{
		// shebang
		if (source[0] == '#' && source[1] == '!')
		{
			source[0] = '/';
			source[1] = '/';
		}

		result = String::NewFromUtf8(isolate, source.c_str(),
			NewStringType::kNormal, source.length());
	}

	return result;
}

loader_impl_data js_loader_impl_initialize(loader_impl impl)
{
	loader_impl_js js_impl = new loader_impl_js_type();

	(void)impl;

	if (js_impl != nullptr)
	{
		if (V8::InitializeICU() == true)
		{
			/* V8::InitializeExternalStartupData(argv[0]); */

			js_impl->platform = platform::CreateDefaultPlatform();

			if (js_impl->platform != nullptr)
			{
				V8::InitializePlatform(js_impl->platform);

				if (V8::Initialize())
				{
					js_impl->isolate_create_params.array_buffer_allocator = &js_impl->allocator;

					js_impl->isolate = Isolate::New(js_impl->isolate_create_params);

					js_impl->isolate_scope = new Isolate::Scope(js_impl->isolate);

					if (js_impl->isolate != nullptr &&
						js_impl->isolate_scope != nullptr)
					{
						return static_cast<loader_impl_data>(js_impl);
					}
				}
			}
		}

		delete js_impl;
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
	loader_impl_js js_impl = static_cast<loader_impl_js>(loader_impl_get(impl));

	(void)name;

	if (js_impl != nullptr)
	{
		loader_impl_js_handle js_handle = new loader_impl_js_handle_type(js_impl, path/*, name*/);

		if (js_handle != nullptr)
		{
			return js_handle;
		}
	}

	return NULL;
}

int js_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_js_handle js_handle = static_cast<loader_impl_js_handle>(handle);

	(void)impl;

	if (js_handle != nullptr)
	{
		delete js_handle;

		return 0;
	}

	return 1;
}

int js_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_js js_impl = static_cast<loader_impl_js>(loader_impl_get(impl));

	if (js_impl != nullptr)
	{
		loader_impl_js_handle js_handle = (loader_impl_js_handle)handle;

		if (js_handle != nullptr)
		{
			return js_handle->discover(js_impl, ctx);
		}
	}

	return 1;
}

int js_loader_impl_destroy(loader_impl impl)
{
	loader_impl_js js_impl = static_cast<loader_impl_js>(loader_impl_get(impl));

	if (js_impl != nullptr)
	{
		if (js_impl->isolate_scope != nullptr)
		{
			delete js_impl->isolate_scope;

			js_impl->isolate_scope = nullptr;
		}

		if (js_impl->isolate != nullptr)
		{
			js_impl->isolate->Dispose();

			js_impl->isolate = nullptr;
		}

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
