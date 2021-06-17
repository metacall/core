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
	jobjectArray handle; // Pointer to the handle JNI object
	size_t size;

} * loader_impl_java_handle;

typedef struct loader_impl_java_class_type
{
	// void *todo;
	jclass cls;
} * loader_impl_java_class;

int java_object_interface_create(object obj, object_impl impl)
{
	(void)obj;

	return 0;
}

value java_object_interface_get(object obj, object_impl impl, const char *key)
{
	(void)obj;

	return NULL;
}

int java_object_interface_set(object obj, object_impl impl, const char *key, value v)
{
	(void)obj;

	return 0;
}

value java_object_interface_method_invoke(object obj, object_impl impl, const char *method_name, object_args args, size_t argc)
{
	(void)obj;

	return NULL;
}

value java_object_interface_method_await(object obj, object_impl impl, const char *key, object_args args, size_t size, object_resolve_callback resolve, object_reject_callback reject, void *ctx)
{
	// TODO
	(void)obj;
	(void)impl;
	(void)key;
	(void)args;
	(void)size;
	(void)resolve;
	(void)reject;
	(void)ctx;

	return NULL;
}

int java_object_interface_destructor(object obj, object_impl impl)
{
	(void)obj;
	(void)impl;

	return 0;
}

void java_object_interface_destroy(object obj, object_impl impl)
{
	(void)obj;
}

object_interface java_object_interface_singleton(void)
{
	static struct object_interface_type java_object_interface = {
		&java_object_interface_create,
		&java_object_interface_get,
		&java_object_interface_set,
		&java_object_interface_method_invoke,
		&java_object_interface_method_await,
		&java_object_interface_destructor,
		&java_object_interface_destroy
	};

	return &java_object_interface;
}

int java_class_interface_create(klass cls, class_impl impl)
{
	(void)cls;

	return 0;
}

object java_class_interface_constructor(klass cls, class_impl impl, const char *name, class_args args, size_t argc)
{
	(void)cls;

	return NULL;
}

value java_class_interface_static_get(klass cls, class_impl impl, const char *key)
{
	(void)cls;

	return NULL;
}

int java_class_interface_static_set(klass cls, class_impl impl, const char *key, value v)
{
	(void)cls;

	return 0;
}

value java_class_interface_static_invoke(klass cls, class_impl impl, const char *static_method_name, class_args args, size_t argc)
{
	// TODO
	(void)cls;
	(void)impl;
	(void)args;

	return NULL;
}

value java_class_interface_static_await(klass cls, class_impl impl, const char *key, class_args args, size_t size, class_resolve_callback resolve, class_reject_callback reject, void *ctx)
{
	// TODO
	(void)cls;
	(void)impl;
	(void)key;
	(void)args;
	(void)size;
	(void)resolve;
	(void)reject;
	(void)ctx;

	return NULL;
}

void java_class_interface_destroy(klass cls, class_impl impl)
{
	(void)cls;
}

class_interface java_class_interface_singleton(void)
{
	static struct class_interface_type java_class_interface = {
		&java_class_interface_create,
		&java_class_interface_constructor,
		&java_class_interface_static_get,
		&java_class_interface_static_set,
		&java_class_interface_static_invoke,
		&java_class_interface_static_await,
		&java_class_interface_destroy
	};

	return &java_class_interface;
}

loader_impl_data java_loader_impl_initialize(loader_impl impl, configuration config)
{
	loader_impl_java java_impl;

	(void)impl;
	(void)config;

	java_impl = new loader_impl_java_type();

	if (java_impl != nullptr)
	{
		// #define TEST_CLASS_PATH
		// 	"$(sbt 'export test:fullClasspath')"

		std::string st = (std::string) "-Djava.class.path=" + getenv("LOADER_LIBRARY_PATH");
		char *javaClassPath = &st[0];

		static const size_t options_size = 2;

		JavaVMOption *options = new JavaVMOption[options_size]; // JVM invocation options
		options[0].optionString = (char *)"-Dmetacall.polyglot.name=core";
		options[1].optionString = javaClassPath;

		log_write("metacall", LOG_LEVEL_ERROR, "Test Log");

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

		log_write("metacall", LOG_LEVEL_ERROR, "JAVA INITIALIZER successful");
		/* Register initialization */
		loader_initialization_register(impl);

		return static_cast<loader_impl_data>(java_impl);
	}

	return NULL;
}

int java_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	// (void)impl;
	// (void)path;

	loader_impl_java java_impl = static_cast<loader_impl_java>(loader_impl_get(impl));
	if (java_impl != NULL)
	{
		jclass classPtr = java_impl->env->FindClass("bootstrap");
		if (classPtr != nullptr)
		{
			jmethodID execPathCall = java_impl->env->GetStaticMethodID(classPtr, "java_bootstrap_execution_path", "(Ljava/lang/String;)I");
			if (execPathCall != nullptr)
			{
				jobject result = java_impl->env->CallObjectMethod(classPtr, execPathCall, java_impl->env->NewStringUTF(path));

				return 0;
			}
		}
	}

	return 1;
}

loader_handle java_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	loader_impl_java_handle java_handle = new loader_impl_java_handle_type();

	if (java_handle != nullptr)
	{
		loader_impl_java java_impl = static_cast<loader_impl_java>(loader_impl_get(impl));
		log_write("metacall", LOG_LEVEL_ERROR, "Load From File");

		jobjectArray arr = java_impl->env->NewObjectArray(size, java_impl->env->FindClass("java/lang/String"), java_impl->env->NewStringUTF(""));

		for (size_t i = 0; i < size; i++) // Create JNI compatible array of paths
		{
			std::cout << "PATH = " << paths[i] << std::endl;
			java_impl->env->SetObjectArrayElement(arr, i, java_impl->env->NewStringUTF(paths[i]));
		}

		jclass classPtr = java_impl->env->FindClass("bootstrap");
		if (classPtr != nullptr)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Bootstrap Found");

			jmethodID mid = java_impl->env->GetStaticMethodID(classPtr, "loadFromFile", "([Ljava/lang/String;)LHandle;");
			if (mid != nullptr)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Function Found");

				jobject result = java_impl->env->CallObjectMethod(classPtr, mid, arr);
				jobjectArray resultArray = java_impl->env->NewObjectArray(size, java_impl->env->FindClass("Handle"), result);

				java_handle->handle = resultArray;
				java_handle->size = size;

				// for (size_t i = 0; i < size + 1; i++)
				// {
				// 	jobject r = java_impl->env->GetObjectArrayElement(rrr, i);
				// 	if (r != nullptr)
				// 		std::cout << "Got it " << i << r << std::endl;
				// }

				java_impl->env->DeleteLocalRef(arr); // Remove the jObjectArray from memory

				return static_cast<loader_handle>(java_handle);
			}
		}
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
	log_write("metacall", LOG_LEVEL_ERROR, "Discover");

	loader_impl_java_handle java_handle = static_cast<loader_impl_java_handle>(handle);
	loader_impl_java java_impl = static_cast<loader_impl_java>(loader_impl_get(impl));

	if (java_handle == NULL || java_impl == NULL || ctx == NULL)
	{
		return 1;
	}

	for (size_t i = 0; i < java_handle->size; i++)
	{
		jobject r = java_impl->env->GetObjectArrayElement(java_handle->handle, i);
		if (r != nullptr)
			std::cout << "Got it " << i << r << std::endl;
	}

	// In functions where handle is passed, we can call it using
	// jobject handle = passes_handle

	return 0;
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
		std::cout << "\n1ffffffffffffffffffffffffffff" << std::endl;

		/* Destroy children loaders */
		loader_unload_children(impl);
		std::cout << "2ffffffffffffffffffffffffffff" << std::endl;

		java_impl->jvm->DestroyJavaVM();

		std::cout << "3ffffffffffffffffffffffffffff" << std::endl;

		delete java_impl;

		return 0;
	}

	return 1;
}
