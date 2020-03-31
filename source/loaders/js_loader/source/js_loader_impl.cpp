/*
 *	Loader Library by Parra Studios
 *	A plugin for loading javascript code at run-time into a process.
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

#include <js_loader/js_loader_impl.h>
#include <js_loader/js_loader_impl_guard.hpp>

#include <loader/loader_impl.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

#include <log/log.h>

#include <cstdlib>
#include <cstring>

#include <new>
#include <string>
#include <fstream>
#include <streambuf>

#include <libplatform/libplatform.h>
#include <v8.h> /* version: 5.1.117 */

#ifdef ENABLE_DEBUGGER_SUPPORT
#	include <v8-debug.h>
#endif /* ENALBLE_DEBUGGER_SUPPORT */

using namespace v8;

MaybeLocal<String> js_loader_impl_read_script(Isolate * isolate, const loader_naming_path path, std::map<std::string, js_function *> & functions);

MaybeLocal<String> js_loader_impl_read_script(Isolate * isolate, const char * buffer, size_t size, std::map<std::string, js_function *> & functions);

void js_loader_impl_obj_to_string(Handle<Value> object, std::string & str);

function_interface function_js_singleton();

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

typedef class loader_impl_js_function_type
{
	public:
		loader_impl_js_function_type(loader_impl_js js_impl, Local<Context> & ctx_impl,
			Isolate * isolate, Local<Function> func) :
			js_impl(js_impl), ctx_impl(ctx_impl),
			isolate_ref(isolate), p_func(isolate, func)
		{

		}

		loader_impl_js get_js_impl()
		{
			return js_impl;
		}

		Local<Context> & get_ctx_impl()
		{
			return ctx_impl;
		}

		Isolate * get_isolate()
		{
			return isolate_ref;
		}

		Local<Function> materialize_handle()
		{
			return Local<Function>::New(isolate_ref, p_func);
		}

		~loader_impl_js_function_type()
		{
			p_func.Reset();
		}

	private:
		loader_impl_js js_impl;
		Local<Context> & ctx_impl;
		Isolate * isolate_ref;
		Persistent<Function> p_func;

} * loader_impl_js_function;

typedef class loader_impl_js_handle_type
{
	public:
		loader_impl_js_handle_type(loader_impl impl, loader_impl_js js_impl,
			const loader_naming_path paths[], size_t size) :
				impl(impl),
				handle_scope(js_impl->isolate),
				ctx_impl(Context::New(js_impl->isolate)), ctx_scope(ctx_impl)
		{
			for (size_t i = 0; i < size; ++i)
			{
				Local<String> source = js_loader_impl_read_script(js_impl->isolate, paths[i], functions).ToLocalChecked();

				script = Script::Compile(ctx_impl, source).ToLocalChecked();

				Local<Value> result = script->Run(ctx_impl).ToLocalChecked();

				String::Utf8Value utf8(result);

				log_write("metacall", LOG_LEVEL_DEBUG, "JS load from file result: %s", *utf8);
			}
		}

		loader_impl_js_handle_type(loader_impl impl, loader_impl_js js_impl,
			const char * buffer, size_t size) :
				impl(impl),
				handle_scope(js_impl->isolate),
				ctx_impl(Context::New(js_impl->isolate)), ctx_scope(ctx_impl)
		{
			Local<String> source = js_loader_impl_read_script(js_impl->isolate, buffer, size, functions).ToLocalChecked();

			script = Script::Compile(ctx_impl, source).ToLocalChecked();

			Local<Value> result = script->Run(ctx_impl).ToLocalChecked();

			String::Utf8Value utf8(result);

			log_write("metacall", LOG_LEVEL_DEBUG, "JS load from file result: %s", *utf8);
		}

		int discover(loader_impl_js js_impl, context ctx)
		{
			Local<Object> global = ctx_impl->Global();

			Local<Array> prop_array = global->GetOwnPropertyNames(ctx_impl).ToLocalChecked();

			return discover_functions(js_impl, ctx, prop_array);
		}

		int discover_functions(loader_impl_js js_impl, context ctx, Local<Array> func_array)
		{
			int length = func_array->Length();

			for (int i = 0; i < length; ++i)
			{
				Local<Value> element = func_array->Get(i);

				Local<Value> func_val;

				if (ctx_impl->Global()->Get(ctx_impl, element).ToLocal(&func_val) &&
					func_val->IsFunction())
				{
					Local<Function> func = Local<Function>::Cast(func_val);

					loader_impl_js_function js_func = new loader_impl_js_function_type(js_impl, ctx_impl, js_impl->isolate, func);

					int arg_count = discover_function_args_count(js_impl, func);

					if (arg_count >= 0)
					{
						std::string func_name, func_obj_name;

						js_loader_impl_obj_to_string(element, func_name);

						js_loader_impl_obj_to_string(func, func_obj_name);

						log_write("metacall", LOG_LEVEL_DEBUG,
							"Function (%d) { %s, %d-arity } => %s",
							i, func_name.c_str(), func_obj_name.c_str());

						function f = function_create(func_name.c_str(),
							arg_count,
							static_cast<void *>(js_func),
							&function_js_singleton);

						if (f != NULL)
						{
							if (discover_function_signature(f) == 0)
							{
								scope sp = context_scope(ctx);

								if (scope_define(sp, function_name(f), f) != 0)
								{
									return 1;
								}
							}
						}
					}
				}
			}

			return 0;
		}

		int discover_function_signature(function f)
		{
			signature s = function_signature(f);

			std::map<std::string, js_function *>::iterator func_it;

			func_it = functions.find(function_name(f));

			if (func_it != functions.end())
			{
				js_function * js_f = func_it->second;

				parameter_list::iterator param_it;

				type ret_type = loader_impl_type(impl, js_f->return_type.c_str());

				signature_set_return(s, ret_type);

				for (param_it = js_f->parameters.begin();
					param_it != js_f->parameters.end(); ++param_it)
				{
					type t = loader_impl_type(impl, param_it->type.c_str());

					signature_set(s, param_it->index, param_it->name.c_str(), t);
				}

				return 0;
			}

			return 1;
		}

		int discover_function_args_count(loader_impl_js js_impl, Local<Function> func)
		{
			Local<String> length_name = String::NewFromUtf8(js_impl->isolate,
				"length", NewStringType::kNormal).ToLocalChecked();

			Local<Value> length_val;

			if (func->Get(ctx_impl, length_name).ToLocal(&length_val)
				&& length_val->IsNumber())
			{
				return length_val->Int32Value();
			}

			return -1;
		}

		~loader_impl_js_handle_type()
		{
			std::map<std::string, js_function *>::iterator it;

			for (it = functions.begin(); it != functions.end(); ++it)
			{
				js_function * js_f = it->second;

				delete js_f;
			}
		}

	private:
		loader_impl impl;
		std::map<std::string, js_function *> functions;
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

function_return function_js_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	loader_impl_js_function js_func = static_cast<loader_impl_js_function>(impl);

	Local<Function> func_impl_local = js_func->materialize_handle();

	signature s = function_signature(func);

	Local<Value> result;

	if (size > 0)
	{
		std::vector<Local<Value>> value_args(size);

		size_t args_count;

		for (args_count = 0; args_count < size; ++args_count)
		{
			type t = signature_get_type(s, args_count);

			type_id id = TYPE_INVALID;

			if (t == NULL)
			{
				id = value_type_id((value)args[args_count]);
			}
			else
			{
				id = type_index(t);
			}

			if (id == TYPE_BOOL)
			{
				boolean * value_ptr = (boolean *)(args[args_count]);

				bool b = (*value_ptr == 1) ? true : false;

				value_args[args_count] = Boolean::New(js_func->get_isolate(), b);
			}
			else if (id == TYPE_INT)
			{
				int * value_ptr = (int *)(args[args_count]);

				value_args[args_count] = Int32::New(js_func->get_isolate(), *value_ptr);
			}
			else if (id == TYPE_LONG)
			{
				long * value_ptr = (long *)(args[args_count]);

				value_args[args_count] = Integer::New(js_func->get_isolate(), *value_ptr);
			}
			else if (id == TYPE_FLOAT)
			{
				float * value_ptr = (float *)(args[args_count]);

				value_args[args_count] = Number::New(js_func->get_isolate(), (double)*value_ptr);
			}
			else if (id == TYPE_DOUBLE)
			{
				double * value_ptr = (double *)(args[args_count]);

				value_args[args_count] = Number::New(js_func->get_isolate(), *value_ptr);
			}
			else if (id == TYPE_STRING)
			{
				const char * value_ptr = (const char *)(args[args_count]);

				Local<String> local_str = String::NewFromUtf8(js_func->get_isolate(),
					value_ptr, NewStringType::kNormal).ToLocalChecked();

				value_args[args_count] = local_str;
			}
			else if (id == TYPE_PTR)
			{
				/*
				void * value_ptr = (void *)(args[args_count]);
				*/

				/* TODO */

				/*
				value_args[args_count] = Number::New(js_func->get_isolate(), *value_ptr);
				*/
			}
			else
			{
				/* handle undefined */
			}
		}

		result = func_impl_local->Call(js_func->get_ctx_impl()->Global(), args_count, &value_args[0]);
	}
	else
	{
		result = func_impl_local->Call(js_func->get_ctx_impl()->Global(), 0, nullptr);
	}

	if (!result->IsUndefined())
	{
		type ret_type = signature_get_return(s);

		type_id id = TYPE_INVALID;

		if (ret_type == NULL)
		{
			if (result->IsNull())
			{
				return NULL;
			}
			else if (result->IsTrue() || result->IsFalse() || result->IsBoolean())
			{
				id = TYPE_BOOL;
			}
			else if (result->IsNumber())
			{
				id = TYPE_DOUBLE;
			}
			else if (result->IsInt32() || result->IsUint32())
			{
				id = TYPE_INT;
			}
			else if (result->IsString())
			{
				id = TYPE_STRING;
			}
		}
		else
		{
			id = type_index(ret_type);
		}

		if (id == TYPE_BOOL)
		{
			bool b = result->BooleanValue();

			boolean bo = (b == true) ? 1 : 0;

			return value_create_bool(bo);
		}
		else if (id == TYPE_CHAR)
		{
			/* TODO: Check signed / unsigned for avoid integer overflow */
			/* TODO: Check rage for avoid integer overflow */
			int i = result->Int32Value();

			return value_create_char((char)i);
		}
		else if (id == TYPE_SHORT)
		{
			/* TODO: Check signed / unsigned for avoid integer overflow */
			/* TODO: Check rage for avoid integer overflow */
			int i = result->Int32Value();

			return value_create_short((short)i);
		}
		else if (id == TYPE_INT)
		{
			/* TODO: Check signed / unsigned for avoid integer overflow */
			int i = result->Int32Value();

			return value_create_int(i);
		}
		else if (id == TYPE_LONG)
		{
			long l = result->IntegerValue();

			return value_create_long(l);
		}
		else if (id == TYPE_FLOAT)
		{
			double d = result->NumberValue();

			return value_create_float((float)d);
		}
		else if (id == TYPE_DOUBLE)
		{
			double d = result->NumberValue();

			return value_create_double(d);
		}
		else if (id == TYPE_STRING)
		{
			Local<String> str_value = result->ToString();

			String::Utf8Value utf8_value(str_value);

			int utf8_length = str_value->Utf8Length();

			if (utf8_length > 0)
			{
				const char * str = *utf8_value;

				size_t length = (size_t)utf8_length;

				return value_create_string(str, length);
			}
		}
		else if (id == TYPE_PTR)
		{
			/* TODO */
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Unrecognized return type");
		}
	}

	return NULL;
}

function_return function_js_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void * context)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)size;
	(void)resolve_callback;
	(void)reject_callback;
	(void)context;

	return NULL;
}

void function_js_interface_destroy(function func, function_impl impl)
{
	loader_impl_js_function js_func = static_cast<loader_impl_js_function>(impl);

	(void)func;

	delete js_func;
}

function_interface function_js_singleton(void)
{
	static struct function_interface_type js_interface =
	{
		&function_js_interface_create,
		&function_js_interface_invoke,
		&function_js_interface_await,
		&function_js_interface_destroy
	};

	return &js_interface;
}

void js_loader_impl_read_file(const loader_naming_path path, std::string & source)
{
	std::ifstream file(path);

	file.seekg(0, std::ios::end);

	source.reserve(file.tellg());

	file.seekg(0, std::ios::beg);

	source.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

MaybeLocal<String> js_loader_impl_read_script(Isolate * isolate, const loader_naming_path path, std::map<std::string, js_function *> & functions)
{
	MaybeLocal<String> result;

	std::string source;

	js_loader_impl_read_file(path, source);

	if (!source.empty())
	{
		std::string output;

		// shebang
		if (source[0] == '#' && source[1] == '!')
		{
			source[0] = '/';
			source[1] = '/';
		}

		if (js_loader_impl_guard_parse(source, functions, output) == true)
		{
			result = String::NewFromUtf8(isolate, output.c_str(),
				NewStringType::kNormal, output.length());
		}
	}

	return result;
}

MaybeLocal<String> js_loader_impl_read_script(Isolate * isolate, const char * buffer, size_t size, std::map<std::string, js_function *> & functions)
{
	MaybeLocal<String> result;

	std::string source(buffer, size - 1);

	if (!source.empty())
	{
		std::string output;

		// shebang
		if (source[0] == '#' && source[1] == '!')
		{
			source[0] = '/';
			source[1] = '/';
		}

		if (js_loader_impl_guard_parse(source, functions, output) == true)
		{
			result = String::NewFromUtf8(isolate, output.c_str(),
				NewStringType::kNormal, output.length());
		}
	}

	return result;
}

void js_loader_impl_obj_to_string(Handle<Value> object, std::string & str)
{
	String::Utf8Value utf8_value(object);

	str = *utf8_value;
}

int js_loader_impl_get_builtin_type(loader_impl impl, loader_impl_js js_impl, type_id id, const char * name)
{
	/* TODO: add type table implementation? */

	type builtin_type = type_create(id, name, NULL, NULL);

	(void)js_impl;

	if (builtin_type != NULL)
	{
		if (loader_impl_type_define(impl, type_name(builtin_type), builtin_type) == 0)
		{
			return 0;
		}

		type_destroy(builtin_type);

	}

	return 1;
}

int js_loader_impl_initialize_inspect_types(loader_impl impl, loader_impl_js js_impl)
{
	/* TODO: move this to loader_impl */

	static struct
	{
		type_id id;
		const char * name;
	}
	type_id_name_pair[] =
	{
		{ TYPE_BOOL, "Boolean" },
		{ TYPE_CHAR, "Int8" },
		{ TYPE_SHORT, "Int16" },
		{ TYPE_INT, "Int32" },
		{ TYPE_LONG, "Integer" },
		{ TYPE_FLOAT, "Float" },
		{ TYPE_DOUBLE, "Number" },
		{ TYPE_STRING, "String" },
		{ TYPE_PTR, "Object" },
		{ TYPE_PTR, "Array" },
		{ TYPE_PTR, "Function" }
	};

	size_t index, size = sizeof(type_id_name_pair) / sizeof(type_id_name_pair[0]);

	for (index = 0; index < size; ++index)
	{
		if (js_loader_impl_get_builtin_type(impl, js_impl,
			type_id_name_pair[index].id,
			type_id_name_pair[index].name) != 0)
		{
				return 1;
		}
	}

	return 0;
}

loader_impl_data js_loader_impl_initialize(loader_impl impl, configuration config, loader_host host)
{
	loader_impl_js js_impl;

	(void)impl;
	(void)config;

	log_copy(host->log);

	js_impl = new loader_impl_js_type();

	if (js_impl != nullptr)
	{
		/* TODO: Implement by config */

		#if 0 /* V8 5.7 */
			V8::InitializeICUDefaultLocation("/../metacall/build", "/../v8/out/x64.debug/icudtl.dat");
			V8::InitializeExternalStartupData("/../v8/out/x64.debug/");
		#else /* V8 5.1 */
			if (V8::InitializeICU() == true)
		#endif
		{
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
						if (js_loader_impl_initialize_inspect_types(impl, js_impl) == 0)
						{
							return static_cast<loader_impl_data>(js_impl);
						}
					}
				}
			}
		}

		delete js_impl;
	}

	return NULL;
}

int js_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	(void)impl;
	(void)path;

	return 0;
}

loader_handle js_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	loader_impl_js js_impl = static_cast<loader_impl_js>(loader_impl_get(impl));

	if (js_impl != nullptr)
	{
		loader_impl_js_handle js_handle = new loader_impl_js_handle_type(impl, js_impl, paths, size);

		if (js_handle != nullptr)
		{
			return js_handle;
		}
	}

	return NULL;
}

loader_handle js_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size)
{
	loader_impl_js js_impl = static_cast<loader_impl_js>(loader_impl_get(impl));

	(void)name;

	if (js_impl != nullptr)
	{
		loader_impl_js_handle js_handle = new loader_impl_js_handle_type(impl, js_impl, buffer, size);

		if (js_handle != nullptr)
		{
			return js_handle;
		}
	}

	return NULL;
}

loader_handle js_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	/* TODO */

	(void)impl;
	(void)path;

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
