/*
 *	Loader Library by Parra Studios
 *	A plugin for loading java code at run-time into a process.
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

#include <java_loader/java_loader_impl.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>

#include <log/log.h>

#include <iostream>

#include <jni.h>

typedef struct loader_impl_java_type
{
	JavaVM *jvm; // Pointer to the JVM (Java Virtual Machine)
	JNIEnv *env; // Pointer to native interface

} * loader_impl_java;

typedef struct loader_impl_java_handle_type
{
	void *todo;

} * loader_impl_java_handle;

typedef struct loader_impl_java_function_type
{
	void *todo;

} * loader_impl_java_function;

//static void java_loader_impl_exception()

int function_java_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

function_return function_java_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)size;

	return NULL;
}

function_return function_java_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
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

void function_java_interface_destroy(function func, function_impl impl)
{
	loader_impl_java_function java_function = static_cast<loader_impl_java_function>(impl);

	(void)func;

	if (java_function != NULL)
	{
		delete java_function;
	}
}

function_interface function_java_singleton()
{
	static struct function_interface_type java_interface = {
		&function_java_interface_create,
		&function_java_interface_invoke,
		&function_java_interface_await,
		&function_java_interface_destroy
	};

	return &java_interface;
}

loader_impl_data java_loader_impl_initialize(loader_impl impl, configuration config, loader_host host)
{
	loader_impl_java java_impl;

	(void)impl;
	(void)config;

	loader_copy(host);

	java_impl = new loader_impl_java_type();

	if (java_impl != nullptr)
	{
#define TEST_CLASS_PATH \
	"$(sbt 'export test:fullClasspath')"

		static const size_t options_size = 2;

		JavaVMOption *options = new JavaVMOption[options_size]; // JVM invocation options
		options[0].optionString = "-Dmetacall.polyglot.name=core";
		options[1].optionString = "-Djava.class.path=" TEST_CLASS_PATH;

		JavaVMInitArgs vm_args;
		vm_args.version = JNI_VERSION_1_6; // Minimum Java version
		vm_args.nOptions = options_size;
		vm_args.options = options;
		vm_args.ignoreUnrecognized = false; // Invalid options make the JVM init fail

		jint rc = JNI_CreateJavaVM(&java_impl->jvm, (void **)&java_impl->env, &vm_args);

		delete[] options;

		if (rc != JNI_OK)
		{
			delete java_impl;
			return NULL;
		}

		/* Register initialization */
		loader_initialization_register(impl);

		return static_cast<loader_impl_data>(java_impl);
	}

	return NULL;
}

int java_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	(void)impl;
	(void)path;

	return 0;
}

loader_handle java_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	loader_impl_java_handle java_handle = new loader_impl_java_handle_type();

	if (java_handle != nullptr)
	{
		loader_impl_java java_impl = static_cast<loader_impl_java>(loader_impl_get(impl));

		jint rc = java_impl->jvm->AttachCurrentThread((void **)&java_impl->env, NULL);

		if (rc != JNI_OK)
		{
			// TODO: Handle error
			std::cout << "ffffffffffffffffffffffffffff" << std::endl;
		}

		// jclass cls2 = java_impl->env->FindClass("metacall/MetaCallSpecRunner");
		jclass cls2 = java_impl->env->FindClass("metacall/CallerSpecRunner");

		if (cls2 == nullptr)
		{
			// TODO: Error handling
			delete java_handle;
			return NULL;
		}

		jmethodID ctor = java_impl->env->GetMethodID(cls2, "<init>", "()V");

		if (cls2 == nullptr)
		{
			// TODO: Error handling
			delete java_handle;
			return NULL;
		}

		jobject myo = java_impl->env->NewObject(cls2, ctor);

		if (myo)
		{
			jmethodID show = java_impl->env->GetMethodID(cls2, "run", "()V");
			if (show == nullptr)
			{
				// TODO: Error handling
				delete java_handle;
				return NULL;
			}
			else
				java_impl->env->CallVoidMethod(myo, show);
		}

		// TODO: Implement a scope like V8 for attaching and detaching automatically
		rc = java_impl->jvm->DetachCurrentThread();

		if (rc != JNI_OK)
		{
			// TODO: Handle error
			std::cout << "333333ffffffffffffffffffffffffffff" << std::endl;
		}

		return static_cast<loader_handle>(java_handle);
	}

	return NULL;
}

loader_handle java_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char *buffer, size_t size)
{
	(void)impl;
	(void)name;
	(void)buffer;
	(void)size;

	return NULL;
}

loader_handle java_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	(void)impl;
	(void)path;

	return NULL;
}

int java_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_java_handle java_handle = static_cast<loader_impl_java_handle>(handle);

	(void)impl;

	if (java_handle != NULL)
	{
		delete java_handle;

		return 0;
	}

	return 1;
}

int java_loader_impl_discover_func(loader_impl impl, loader_handle handle, context ctx, function f)
{
	(void)impl;
	(void)handle;
	(void)ctx;
	(void)f;

	return 0;
}

int java_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_java_handle java_handle = static_cast<loader_impl_java_handle>(handle);

	(void)impl;
	(void)ctx;

	if (java_handle != NULL)
	{
		return 0;
	}

	return 1;
}

int java_loader_impl_destroy(loader_impl impl)
{
	loader_impl_java java_impl = static_cast<loader_impl_java>(loader_impl_get(impl));

	if (java_impl != NULL)
	{
		jint rc = java_impl->jvm->AttachCurrentThread((void **)&java_impl->env, NULL);

		if (rc != JNI_OK)
		{
			// TODO: Handle error
			std::cout << "ffffffffffffffffffffffffffff" << std::endl;
		}
		std::cout << "1ffffffffffffffffffffffffffff" << std::endl;

		/* Destroy children loaders */
		loader_unload_children();
		std::cout << "2ffffffffffffffffffffffffffff" << std::endl;

		java_impl->jvm->DestroyJavaVM();

		std::cout << "3ffffffffffffffffffffffffffff" << std::endl;

		delete java_impl;

		return 0;
	}

	return 1;
}
