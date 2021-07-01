/*
make metacall-java-test && make java_loader && ctest -VV -R metacall-java-test
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

#include <string.h>
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
	size_t size;		 // Size of the jobject array

} * loader_impl_java_handle;

typedef struct loader_impl_java_class_type
{
	const char *name;
	jobject cls;
	loader_impl_java impl;
} * loader_impl_java_class;

typedef struct loader_impl_java_object_type
{
	jobject cls;
	loader_impl_java impl;
} * loader_impl_java_object;

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

value java_loader_capi_to_value(const char *val, const char *type)
{
	std::cout << val << " " << type << std::endl;

	value v = NULL;

	if (!strcmp(type, "int"))
	{
		std::cout << "INT" << std::endl;
		int intVal = std::stoi(val);

		v = value_create_int(intVal);
	}
	else if (!strcmp(type, "java.lang.String"))
	{
		std::cout << "STRING" << std::endl;

		v = value_create_string(val, (size_t)strlen(val));
	}

	return v;
}

int java_class_interface_create(klass cls, class_impl impl)
{
	(void)cls;
	std::cout << "Create" << std::endl;
	loader_impl_java_class java_cls = (loader_impl_java_class)impl;
	// java_cls->cls = NULL;

	return 0;
}

object java_class_interface_constructor(klass cls, class_impl impl, const char *name, class_args args, size_t argc)
{
	(void)cls;

	std::cout << "Constructor " << argc << std::endl;

	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);

	loader_impl_java_object java_obj = new loader_impl_java_object_type();

	object obj = object_create(name, java_obj, &java_object_interface_singleton, cls);

	if (obj == NULL)
		return NULL;

	java_obj->impl = java_cls->impl;
	jobject clsObj = java_cls->cls;

	jvalue constructorArgs[argc];

	for (int i = 0; i < argc; i++)
	{
		constructorArgs[i].i = value_to_int(args[i]);
		std::cout << constructorArgs[i].i << std::endl;
	}

	// if (java_cls->impl->env != nullptr && java_cls->cls != nullptr)
	// {
	// 	jclass classPtr = java_cls->impl->env->FindClass("Test");

	// 	if (classPtr != nullptr)
	// 	{
	// 		jmethodID cls_call_constructor = java_cls->impl->env->GetStaticMethodID(classPtr, "java_bootstrap_call_constructor", "([Ljava/lang/Class;)Ljava/lang/String;");

	// 		if (cls_call_constructor != nullptr)
	// 		{
	// 			jstring result = (jstring)java_cls->impl->env->CallStaticObjectMethodA(classPtr, cls_call_constructor, jvalue * constructorArgs);
	// 			const char *cls_name = java_cls->impl->env->GetStringUTFChars(result, NULL);
	// 		}
	// }
	// }

	return obj;
}

value java_class_interface_static_get(klass cls, class_impl impl, const char *key)
{
	(void)cls;
	std::cout << "\nGet -> " << key << std::endl;

	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);
	loader_impl_java java_impl = java_cls->impl;

	jobject clsObj = java_cls->cls;
	jstring getKey = java_cls->impl->env->NewStringUTF(key);

	jclass classPtr = java_cls->impl->env->FindClass("bootstrap");

	if (classPtr != nullptr)
	{
		jmethodID cls_get_field_type = java_cls->impl->env->GetStaticMethodID(classPtr, "get_Field_Type", "(Ljava/lang/Class;Ljava/lang/String;)Ljava/lang/String;");

		if (cls_get_field_type != nullptr)
		{
			jstring result = (jstring)java_impl->env->CallStaticObjectMethod(classPtr, cls_get_field_type, clsObj, getKey);
			const char *gotType = java_impl->env->GetStringUTFChars(result, NULL);
			std::cout << "GOT Field Type = " << gotType << std::endl;

			//Test function for using templates
			// if (!strcmp(gotType, "int"))
			// {
			// 	jmethodID cls_get_val = java_cls->impl->env->GetStaticMethodID(classPtr, "java_bootstrap_get_temp_value", "(Ljava/lang/Class;Ljava/lang/String;)Ljava/lang/Object;");
			// 	if (cls_get_val != nullptr)
			// 	{
			// 		jint intResult = (jint)java_impl->env->CallStaticIntMethod(classPtr, cls_get_val, clsObj, getKey);
			// 		std::cout << "GOT Int = " << intResult << std::endl;
			// 		return value_create_int(30);
			// 	}
			// }

			if (!strcmp(gotType, "int"))
			{
				jmethodID cls_get_val = java_cls->impl->env->GetStaticMethodID(classPtr, "java_bootstrap_get_int_value", "(Ljava/lang/Class;Ljava/lang/String;)I");
				if (cls_get_val != nullptr)
				{
					jint intResult = (jint)java_impl->env->CallStaticIntMethod(classPtr, cls_get_val, clsObj, getKey);
					std::cout << "GOT Int = " << intResult << std::endl;
					return value_create_int(intResult);
				}
			}
			else if (!strcmp(gotType, "char"))
			{
				jmethodID cls_get_val = java_cls->impl->env->GetStaticMethodID(classPtr, "java_bootstrap_get_char_value", "(Ljava/lang/Class;Ljava/lang/String;)C");
				if (cls_get_val != nullptr)
				{
					jchar charResult = (jchar)java_impl->env->CallStaticCharMethod(classPtr, cls_get_val, clsObj, getKey);
					std::cout << "GOT Char = " << charResult << std::endl;
					return value_create_char(charResult);
				}
			}
			else if (!strcmp(gotType, "java.lang.String"))
			{
				jmethodID cls_get_val = java_cls->impl->env->GetStaticMethodID(classPtr, "java_bootstrap_get_string_value", "(Ljava/lang/Class;Ljava/lang/String;)Ljava/lang/String;");
				if (cls_get_val != nullptr)
				{
					jstring stringResult = (jstring)java_impl->env->CallStaticObjectMethod(classPtr, cls_get_val, clsObj, getKey);
					const char *stringR = java_impl->env->GetStringUTFChars(stringResult, NULL);
					std::cout << "GOT String = " << stringR << std::endl;
					return value_create_string(stringR, (size_t)strlen(stringR));
				}
			}
		}
	}

	return NULL;
}

int java_class_interface_static_set(klass cls, class_impl impl, const char *key, value v)
{
	(void)cls;
	std::cout << "\nSet = " << key << std::endl;

	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);
	loader_impl_java java_impl = java_cls->impl;

	jobject clsObj = java_cls->cls;
	jstring setKey = java_cls->impl->env->NewStringUTF(key);

	jclass classPtr = java_cls->impl->env->FindClass("bootstrap");

	if (classPtr != nullptr)
	{
		type_id id = value_type_id(v);

		if (id == TYPE_INT)
		{
			int i = value_to_int(v);

			jmethodID cls_set_val = java_cls->impl->env->GetStaticMethodID(classPtr, "java_bootstrap_set_int_value", "(Ljava/lang/Class;Ljava/lang/String;I)I");
			if (cls_set_val != nullptr)
			{
				jint intResult = (jint)java_impl->env->CallStaticIntMethod(classPtr, cls_set_val, clsObj, setKey, i);
				return intResult;
			}
		}
	}

	return 0;
}

value java_class_interface_static_invoke(klass cls, class_impl impl, const char *static_method_name, class_args args, size_t argc)
{
	// TODO
	(void)cls;
	(void)impl;
	(void)args;

	std::cout << "invoke" << std::endl;

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
	std::cout << "Destroy" << std::endl;
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

			jmethodID mid = java_impl->env->GetStaticMethodID(classPtr, "loadFromFile", "([Ljava/lang/String;)[Ljava/lang/Class;");
			if (mid != nullptr)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Function Found");

				jobjectArray result = (jobjectArray)java_impl->env->CallStaticObjectMethod(classPtr, mid, arr);

				java_handle->handle = result;
				java_handle->size = java_impl->env->GetArrayLength(result);

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

	jclass classPtr = java_impl->env->FindClass("bootstrap");

	if (classPtr != nullptr)
	{
		jmethodID cls_name_bootstrap = java_impl->env->GetStaticMethodID(classPtr, "java_bootstrap_get_class_name", "(Ljava/lang/Class;)Ljava/lang/String;");

		if (cls_name_bootstrap != nullptr)
		{
			size_t handleSize = (size_t)java_impl->env->GetArrayLength(java_handle->handle);

			for (size_t i = 0; i < handleSize; i++)
			{
				jobject r = java_impl->env->GetObjectArrayElement(java_handle->handle, i);

				if (r != nullptr)
				{
					jstring result = (jstring)java_impl->env->CallStaticObjectMethod(classPtr, cls_name_bootstrap, r);
					const char *cls_name = java_impl->env->GetStringUTFChars(result, NULL);

					loader_impl_java_class java_cls = new loader_impl_java_class_type();

					java_cls->name = cls_name;
					java_cls->cls = r;
					java_cls->impl = (loader_impl_java)java_impl;

					klass c = class_create(cls_name, java_cls, &java_class_interface_singleton);

					// if (py_loader_impl_discover_class(impl, module_dict_val, c) == 0)
					// {
					scope sp = context_scope(ctx);
					scope_define(sp, cls_name, value_create_class(c));

					std::cout << cls_name << " Class Registered!\n"
							  << std::endl;

					// }
					// else
					// {
					// 	class_destroy(c);
					// }
				}

				// java_impl->env->DeleteLocalRef(r); // Remove the jObjectArray element from memory
			}
		}
	}

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
