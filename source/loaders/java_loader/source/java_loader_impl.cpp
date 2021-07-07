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
	jclass concls;
	loader_impl_java impl;
} * loader_impl_java_class;

typedef struct loader_impl_java_object_type
{
	const char *name;
	jobject conObj;
	jclass concls;
	loader_impl_java impl;
} * loader_impl_java_object;

std::string get_val_sig(const char *type)
{
	if (!strcmp(type, "boolean"))
		return "Z";
	if (!strcmp(type, "char"))
		return "C";
	if (!strcmp(type, "short"))
		return "S";
	if (!strcmp(type, "int"))
		return "I";
	if (!strcmp(type, "long"))
		return "J";
	if (!strcmp(type, "float"))
		return "F";
	if (!strcmp(type, "double"))
		return "D";
	if (!strcmp(type, "java.lang.String"))
		return "Ljava/lang/String;";

	return "V";
}

int java_object_interface_create(object obj, object_impl impl)
{
	(void)obj;
	std::cout << "Obj Create" << std::endl;
	loader_impl_java_object java_obj = (loader_impl_java_object)impl;

	return 0;
}

value java_object_interface_get(object obj, object_impl impl, const char *key)
{
	(void)obj;
	std::cout << "Obj Get = " << key << std::endl;

	loader_impl_java_object java_obj = static_cast<loader_impl_java_object>(impl);
	loader_impl_java java_impl = java_obj->impl;

	jstring getKey = java_impl->env->NewStringUTF(key);

	jobject conobj = java_obj->conObj;
	jclass concls = java_obj->concls;

	jclass classPtr = java_impl->env->FindClass("bootstrap");
	if (classPtr != nullptr)
	{
		jmethodID cls_get_field_type = java_impl->env->GetStaticMethodID(classPtr, "get_Field_Type", "(Ljava/lang/Class;Ljava/lang/String;)Ljava/lang/String;");

		if (cls_get_field_type != nullptr)
		{
			jstring result = (jstring)java_impl->env->CallStaticObjectMethod(classPtr, cls_get_field_type, concls, getKey);
			const char *gotType = java_impl->env->GetStringUTFChars(result, NULL); // Field Type for get key
			std::cout << "GOT Obj Field Type = " << gotType << std::endl;

			if (concls != nullptr && conobj != nullptr)
			{
				jfieldID fID = java_impl->env->GetFieldID(concls, key, get_val_sig(gotType).c_str());

				if (fID != nullptr)
				{
					if (!strcmp(gotType, "boolean"))
					{
						jboolean gotVal = (jboolean)java_impl->env->GetBooleanField(conobj, fID);
						return value_create_bool(gotVal);
					}
					if (!strcmp(gotType, "char"))
					{
						jchar gotVal = (jchar)java_impl->env->GetCharField(conobj, fID);
						return value_create_char(gotVal);
					}
					if (!strcmp(gotType, "short"))
					{
						jshort gotVal = (jshort)java_impl->env->GetShortField(conobj, fID);
						return value_create_short(gotVal);
					}
					if (!strcmp(gotType, "int"))
					{
						jint gotVal = (jint)java_impl->env->GetIntField(conobj, fID);
						return value_create_int(gotVal);
					}
					if (!strcmp(gotType, "long"))
					{
						jlong gotVal = (jlong)java_impl->env->GetLongField(conobj, fID);
						return value_create_long(gotVal);
					}
					if (!strcmp(gotType, "float"))
					{
						jfloat gotVal = (jfloat)java_impl->env->GetFloatField(conobj, fID);
						return value_create_float(gotVal);
					}
					if (!strcmp(gotType, "double"))
					{
						jdouble gotVal = (jdouble)java_impl->env->GetDoubleField(conobj, fID);
						return value_create_double(gotVal);
					}
					if (!strcmp(gotType, "java.lang.String"))
					{
						jstring gotVal = (jstring)java_impl->env->GetObjectField(conobj, fID);
						const char *gotValConv = java_impl->env->GetStringUTFChars(gotVal, NULL);
						return value_create_string(gotValConv, strlen(gotValConv));
					}
				}
			}
		}
	}

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

std::string getJNISignature(class_args args, size_t argc, const char *returnType)
{
	std::string sig = "(";

	for (int i = 0; i < argc; i++)
	{
		type_id id = value_type_id(args[i]);

		if (id == TYPE_BOOL)
			sig += "Z";

		if (id == TYPE_CHAR)
			sig += "C";

		if (id == TYPE_SHORT)
			sig += "S";

		if (id == TYPE_INT)
			sig += "I";

		if (id == TYPE_LONG)
			sig += "J";

		if (id == TYPE_FLOAT)
			sig += "F";

		if (id == TYPE_DOUBLE)
			sig += "D";

		if (id == TYPE_STRING)
			sig += "Ljava/lang/String;";
	}

	sig += ")";

	if (!strcmp(returnType, "void"))
		sig += "V";

	return sig;
}

void getJValArray(jvalue *constructorArgs, class_args args, size_t argc, JNIEnv *env)
{
	for (int i = 0; i < argc; i++)
	{
		type_id id = value_type_id(args[i]);

		if (id == TYPE_BOOL)
		{
			constructorArgs[i].z = value_to_bool(args[i]);
			std::string ss = constructorArgs[i].z ? "1" : "0";
			std::cout << "BOOL = " << ss << std::endl;
		}
		else if (id == TYPE_CHAR)
		{
			constructorArgs[i].c = value_to_char(args[i]);
			std::cout << "CHAR = " << constructorArgs[i].c << std::endl;
		}
		else if (id == TYPE_SHORT)
		{
			constructorArgs[i].s = value_to_short(args[i]);
			std::cout << "SHORT = " << constructorArgs[i].s << std::endl;
		}
		else if (id == TYPE_INT)
		{
			constructorArgs[i].i = value_to_int(args[i]);
			std::cout << "INT = " << constructorArgs[i].i << std::endl;
		}
		else if (id == TYPE_LONG)
		{
			constructorArgs[i].j = value_to_long(args[i]);
			std::cout << "LONG = " << constructorArgs[i].j << std::endl;
		}
		else if (id == TYPE_FLOAT)
		{
			constructorArgs[i].f = value_to_float(args[i]);
			std::cout << "FLOAT = " << constructorArgs[i].f << std::endl;
		}
		else if (id == TYPE_DOUBLE)
		{
			constructorArgs[i].d = value_to_double(args[i]);
			std::cout << "DOUBLE = " << constructorArgs[i].d << std::endl;
		}
		else if (id == TYPE_STRING)
		{
			const char *strV = value_to_string(args[i]);
			jstring str = env->NewStringUTF(strV);
			constructorArgs[i].l = str;
		}
	}
}

int java_class_interface_create(klass cls, class_impl impl)
{
	(void)cls;
	std::cout << "Create" << std::endl;
	loader_impl_java_class java_cls = (loader_impl_java_class)impl;

	return 0;
}

object java_class_interface_constructor(klass cls, class_impl impl, const char *name, class_args args, size_t argc)
{
	(void)cls;
	std::cout << "Constructor" << std::endl;

	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);
	loader_impl_java_object java_obj = new loader_impl_java_object_type();

	object obj = object_create(name, java_obj, &java_object_interface_singleton, cls);

	if (obj == NULL)
		return NULL;

	java_obj->impl = java_cls->impl;

	jvalue constructorArgs[argc];
	getJValArray(constructorArgs, args, argc, java_cls->impl->env); // Create a jvalue array that can be passed to JNI

	if (java_cls->impl->env != nullptr && java_cls->cls != nullptr)
	{
		jclass classPtr = java_cls->impl->env->FindClass("bootstrap");

		if (classPtr != nullptr)
		{
			jmethodID findCls = java_cls->impl->env->GetStaticMethodID(classPtr, "FindClass", "(Ljava/lang/String;)Ljava/lang/Class;");
			if (findCls != nullptr)
			{
				jclass clscls = (jclass)java_cls->impl->env->CallStaticObjectMethod(classPtr, findCls, java_cls->impl->env->NewStringUTF(name));
				if (clscls != nullptr)
				{
					std::string sig = getJNISignature(args, argc, "void");

					jmethodID constMID = java_cls->impl->env->GetMethodID(clscls, "<init>", sig.c_str());
					if (constMID != nullptr)
					{
						jobject newCls = java_cls->impl->env->NewObjectA(clscls, constMID, constructorArgs);
						if (newCls != nullptr)
						{
							std::cout << "Class Constructor successful\n\n";
							java_cls->concls = clscls;

							java_obj->concls = clscls;
							java_obj->conObj = newCls;
							java_obj->name = name;
						}
					}
				}
			}
		}
	}

	return obj;
}

value java_class_interface_static_get(klass cls, class_impl impl, const char *key)
{
	(void)cls;
	std::cout << "\nGet -> " << key << std::endl;

	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);
	loader_impl_java java_impl = java_cls->impl;

	jobject clsObj = java_cls->cls;
	jstring getKey = java_impl->env->NewStringUTF(key);

	jclass classPtr = java_impl->env->FindClass("bootstrap");

	if (classPtr != nullptr)
	{
		jmethodID cls_get_field_type = java_impl->env->GetStaticMethodID(classPtr, "get_Field_Type", "(Ljava/lang/Class;Ljava/lang/String;)Ljava/lang/String;");

		if (cls_get_field_type != nullptr)
		{
			jstring result = (jstring)java_impl->env->CallStaticObjectMethod(classPtr, cls_get_field_type, clsObj, getKey);
			const char *gotType = java_impl->env->GetStringUTFChars(result, NULL); // Field Type for get key
			std::cout << "GOT Field Type = " << gotType << std::endl;
			jclass clscls = java_cls->concls;
			if (clscls != nullptr)
			{
				jfieldID fID = java_impl->env->GetStaticFieldID(clscls, key, get_val_sig(gotType).c_str());

				if (fID != nullptr)
				{
					if (!strcmp(gotType, "boolean"))
					{
						jboolean gotVal = (jboolean)java_impl->env->GetStaticBooleanField(clscls, fID);
						return value_create_bool(gotVal);
					}
					if (!strcmp(gotType, "char"))
					{
						jchar gotVal = (jchar)java_impl->env->GetStaticCharField(clscls, fID);
						return value_create_char(gotVal);
					}
					if (!strcmp(gotType, "short"))
					{
						jshort gotVal = (jshort)java_impl->env->GetStaticShortField(clscls, fID);
						return value_create_short(gotVal);
					}
					if (!strcmp(gotType, "int"))
					{
						jint gotVal = (jint)java_impl->env->GetStaticIntField(clscls, fID);
						return value_create_int(gotVal);
					}
					if (!strcmp(gotType, "long"))
					{
						jlong gotVal = (jlong)java_impl->env->GetStaticLongField(clscls, fID);
						return value_create_long(gotVal);
					}
					if (!strcmp(gotType, "float"))
					{
						jfloat gotVal = (jfloat)java_impl->env->GetStaticFloatField(clscls, fID);
						return value_create_float(gotVal);
					}
					if (!strcmp(gotType, "double"))
					{
						jdouble gotVal = (jdouble)java_impl->env->GetStaticDoubleField(clscls, fID);
						return value_create_double(gotVal);
					}
					if (!strcmp(gotType, "java.lang.String"))
					{
						jstring gotVal = (jstring)java_impl->env->GetStaticObjectField(clscls, fID);
						const char *gotValConv = java_impl->env->GetStringUTFChars(gotVal, NULL);
						return value_create_string(gotValConv, strlen(gotValConv));
					}
				}
			}
		}
	}

	return NULL;
}

std::string get_val_sig(type_id type)
{
	if (type == TYPE_BOOL)
		return "Z";
	if (type == TYPE_CHAR)
		return "C";
	if (type == TYPE_SHORT)
		return "S";
	if (type == TYPE_INT)
		return "I";
	if (type == TYPE_LONG)
		return "J";
	if (type == TYPE_FLOAT)
		return "F";
	if (type == TYPE_DOUBLE)
		return "D";
	if (type == TYPE_STRING)
		return "Ljava/lang/String;";

	return "V";
}

int java_class_interface_static_set(klass cls, class_impl impl, const char *key, value v)
{
	(void)cls;
	std::cout << "\nSet = " << key << std::endl;

	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);
	loader_impl_java java_impl = java_cls->impl;

	jobject clsObj = java_cls->cls;

	jclass clscls = java_cls->concls;
	if (clscls != nullptr)
	{
		type_id id = value_type_id(v);
		jfieldID fID = java_cls->impl->env->GetStaticFieldID(clscls, key, get_val_sig(id).c_str());

		if (fID != nullptr)
		{
			if (id == TYPE_BOOL)
			{
				jboolean val = (jboolean)value_to_bool(v);
				java_impl->env->SetStaticBooleanField(clscls, fID, val);
				return 0;
			}
			if (id == TYPE_CHAR)
			{
				jchar val = (jchar)value_to_char(v);
				java_impl->env->SetStaticCharField(clscls, fID, val);
				return 0;
			}
			if (id == TYPE_SHORT)
			{
				jshort val = (jshort)value_to_short(v);
				java_impl->env->SetStaticShortField(clscls, fID, val);
				return 0;
			}
			if (id == TYPE_INT)
			{
				jint val = (jint)value_to_int(v);
				java_impl->env->SetStaticIntField(clscls, fID, val);
				return 0;
			}
			if (id == TYPE_LONG)
			{
				jlong val = (jlong)value_to_long(v);
				java_impl->env->SetStaticLongField(clscls, fID, val);
				return 0;
			}
			if (id == TYPE_FLOAT)
			{
				jfloat val = (jfloat)value_to_float(v);
				java_impl->env->SetStaticFloatField(clscls, fID, val);
				return 0;
			}
			if (id == TYPE_DOUBLE)
			{
				jdouble val = (jdouble)value_to_double(v);
				java_impl->env->SetStaticDoubleField(clscls, fID, val);
				return 0;
			}
			if (id == TYPE_STRING)
			{
				const char *strV = value_to_string(v);
				jstring val = java_impl->env->NewStringUTF(strV);
				java_impl->env->SetStaticObjectField(clscls, fID, val);
				return 0;
			}
		}
	}

	return 1;
}

value java_class_interface_static_invoke(klass cls, class_impl impl, const char *static_method_name, class_args args, size_t argc)
{
	// IN PROGRESS
	(void)cls;
	std::cout << "invoke" << std::endl;

	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);

	loader_impl_java java_impl = java_cls->impl;
	jclass clscls = java_cls->concls;

	jobject clsObj = java_cls->cls;
	jstring getKey = java_cls->impl->env->NewStringUTF(static_method_name);

	jclass classPtr = java_cls->impl->env->FindClass("bootstrap");
	if (classPtr != nullptr)
	{
		jmethodID cls_static_return_type = java_cls->impl->env->GetStaticMethodID(classPtr, "get_static_invoke_return_type", "(Ljava/lang/Class;Ljava/lang/String;)Ljava/lang/String;");
		if (cls_static_return_type != nullptr)
		{
			jstring result = (jstring)java_impl->env->CallStaticObjectMethod(classPtr, cls_static_return_type, clsObj, getKey);
			const char *rtnType = java_impl->env->GetStringUTFChars(result, NULL);
			std::cout << "Static Return type = " << rtnType << std::endl;
		}
	}

	// jmethodID findCls = java_impl->env->GetStaticMethodID(classPtr, "FindClass", "(Ljava/lang/String;)Ljava/lang/Class;");
	// if (findCls != nullptr)
	// {
	// 	jclass clscls = (jclass)java_impl->env->CallStaticObjectMethod(classPtr, findCls, java_impl->env->NewStringUTF("Test"));
	// 	if (clscls != nullptr)
	// 	{
	// 		jmethodID testFunct = java_impl->env->GetStaticMethodID(clscls, "testFunct", "(Ljava/lang/String;)I");
	// 		if (testFunct != nullptr)
	// 		{
	// 			std::cout << "Got " << std::endl;

	// 			jint clsrtn = (jint)java_impl->env->CallStaticIntMethod(clscls, testFunct, java_impl->env->NewStringUTF("Mr. stark"));
	// 			std::cout << "We won mr. stark " << clsrtn << std::endl;
	// 		}
	// 	}
	// }

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
		std::string st = (std::string) "-Djava.class.path=" + getenv("LOADER_LIBRARY_PATH");
		char *javaClassPath = &st[0];

		static const size_t options_size = 2;

		JavaVMOption *options = new JavaVMOption[options_size]; // JVM invocation options
		options[0].optionString = (char *)"-Dmetacall.polyglot.name=core";
		options[1].optionString = javaClassPath;

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

		std::cout << "\nJAVA INITIALIZER successful\n";
		/* Register initialization */
		loader_initialization_register(impl);

		return static_cast<loader_impl_data>(java_impl);
	}

	return NULL;
}

int java_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	loader_impl_java java_impl = static_cast<loader_impl_java>(loader_impl_get(impl));
	if (java_impl != NULL)
	{
		jclass classPtr = java_impl->env->FindClass("bootstrap");
		if (classPtr != nullptr)
		{
			jmethodID execPathCall = java_impl->env->GetStaticMethodID(classPtr, "java_bootstrap_execution_path", "(Ljava/lang/String;)I");
			if (execPathCall != nullptr)
			{
				jint result = (jint)java_impl->env->CallStaticIntMethod(classPtr, execPathCall, java_impl->env->NewStringUTF(path));
				return result;
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
