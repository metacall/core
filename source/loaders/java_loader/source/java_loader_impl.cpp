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

#include <reflect/reflect_attribute.h>
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

typedef struct loader_impl_java_field_type
{
	const char *fieldName;
	jobject fieldObj;
} * loader_impl_java_field;

typedef struct loader_impl_java_method_type
{
	jobject methodObj;
	const char *methodSignature;
} * loader_impl_java_method;

std::string get_val_sig(const char *type)
{
	// TODO: Remove this (this will be not necessary because it will be handled with type_impl)

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

std::string get_val_sig(type_id type)
{
	// TODO: Remove this (this will be not necessary because it will be handled with type_impl)

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

int type_java_interface_create(type t, type_impl impl)
{
	(void)t;
	(void)impl;

	return 0;
}

void type_java_interface_destroy(type t, type_impl impl)
{
	(void)t;
	(void)impl;
}

type_interface type_java_singleton(void)
{
	static struct type_interface_type java_type_interface = {
		&type_java_interface_create,
		&type_java_interface_destroy
	};

	return &java_type_interface;
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
				// TODO: Remove get_val_sig
				jfieldID fID = java_impl->env->GetFieldID(concls, key, get_val_sig(gotType).c_str());

				if (fID != nullptr)
				{
					// TODO: Convert this to switch with type_id
					if (!strcmp(gotType, "boolean"))
					{
						jboolean gotVal = java_impl->env->GetBooleanField(conobj, fID);
						return value_create_bool((boolean)gotVal);
					}
					if (!strcmp(gotType, "char"))
					{
						jchar gotVal = java_impl->env->GetCharField(conobj, fID);
						return value_create_char((char)gotVal);
					}
					if (!strcmp(gotType, "short"))
					{
						jshort gotVal = java_impl->env->GetShortField(conobj, fID);
						return value_create_short((short)gotVal);
					}
					if (!strcmp(gotType, "int"))
					{
						jint gotVal = java_impl->env->GetIntField(conobj, fID);
						return value_create_int((int)gotVal);
					}
					if (!strcmp(gotType, "long"))
					{
						jlong gotVal = java_impl->env->GetLongField(conobj, fID);
						return value_create_long((long)gotVal);
					}
					if (!strcmp(gotType, "float"))
					{
						jfloat gotVal = java_impl->env->GetFloatField(conobj, fID);
						return value_create_float((float)gotVal);
					}
					if (!strcmp(gotType, "double"))
					{
						jdouble gotVal = java_impl->env->GetDoubleField(conobj, fID);
						return value_create_double((double)gotVal);
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
	std::cout << "\nObj Set = " << key << std::endl;

	loader_impl_java_object java_obj = static_cast<loader_impl_java_object>(impl);
	loader_impl_java java_impl = java_obj->impl;

	jobject conobj = java_obj->conObj;
	jclass concls = java_obj->concls;

	if (concls != nullptr && conobj != nullptr)
	{
		type_id id = value_type_id(v);
		// TODO: Remove get_val_sig
		jfieldID fID = java_impl->env->GetFieldID(concls, key, get_val_sig(id).c_str());

		if (fID != nullptr)
		{
			if (id == TYPE_BOOL)
			{
				jboolean val = (jboolean)value_to_bool(v);
				java_impl->env->SetBooleanField(conobj, fID, val);
				return 0;
			}
			if (id == TYPE_CHAR)
			{
				jchar val = (jchar)value_to_char(v);
				java_impl->env->SetCharField(conobj, fID, val);
				return 0;
			}
			if (id == TYPE_SHORT)
			{
				jshort val = (jshort)value_to_short(v);
				java_impl->env->SetShortField(conobj, fID, val);
				return 0;
			}
			if (id == TYPE_INT)
			{
				jint val = (jint)value_to_int(v);
				java_impl->env->SetIntField(conobj, fID, val);
				return 0;
			}
			if (id == TYPE_LONG)
			{
				jlong val = (jlong)value_to_long(v);
				java_impl->env->SetLongField(conobj, fID, val);
				return 0;
			}
			if (id == TYPE_FLOAT)
			{
				jfloat val = (jfloat)value_to_float(v);
				java_impl->env->SetFloatField(conobj, fID, val);
				return 0;
			}
			if (id == TYPE_DOUBLE)
			{
				jdouble val = (jdouble)value_to_double(v);
				java_impl->env->SetDoubleField(conobj, fID, val);
				return 0;
			}
			if (id == TYPE_STRING)
			{
				const char *strV = value_to_string(v);
				jstring val = java_impl->env->NewStringUTF(strV);
				java_impl->env->SetObjectField(conobj, fID, val);
				return 0;
			}
		}
	}

	return 1;
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

	for (size_t i = 0; i < argc; i++)
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
	for (size_t i = 0; i < argc; i++)
	{
		type_id id = value_type_id(args[i]);

		if (id == TYPE_BOOL)
		{
			constructorArgs[i].z = value_to_bool(args[i]);
			std::string ss = constructorArgs[i].z ? "1" : "0";
			std::cout << "BOOL = " << ss.c_str() << std::endl;
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

	jvalue *constructorArgs = nullptr;

	if (argc > 0)
	{
		constructorArgs = new jvalue[argc];
		getJValArray(constructorArgs, args, argc, java_cls->impl->env); // Create a jvalue array that can be passed to JNI
	}

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

	if (constructorArgs != nullptr)
	{
		delete[] constructorArgs;
	}

	return obj;
}

value java_class_interface_static_get(klass cls, class_impl impl, attribute attr)
{
	(void)cls;

	const char *key = (const char *)attribute_name(attr);
	type fieldType = (type)attribute_type(attr);

	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);
	loader_impl_java java_impl = java_cls->impl;

	jobject clsObj = java_cls->cls;
	jclass clscls = java_cls->concls;

	jstring getKey = java_impl->env->NewStringUTF(key);

	if (clscls != nullptr)
	{
		const char *fType = (const char *)type_derived(fieldType);
		jfieldID fID = java_impl->env->GetStaticFieldID(clscls, key, fType);

		if (fID != nullptr)
		{
			type_id id = type_index(fieldType);

			switch (id)
			{
				case TYPE_BOOL: {
					jboolean gotVal = java_impl->env->GetStaticBooleanField(clscls, fID);
					return value_create_bool((boolean)gotVal);
				}

				case TYPE_CHAR: {
					jchar gotVal = java_impl->env->GetStaticCharField(clscls, fID);
					return value_create_char((char)gotVal);
				}

				case TYPE_SHORT: {
					jshort gotVal = java_impl->env->GetStaticShortField(clscls, fID);
					return value_create_short((short)gotVal);
				}

				case TYPE_INT: {
					jint gotVal = java_impl->env->GetStaticIntField(clscls, fID);
					return value_create_int((int)gotVal);
				}

				case TYPE_LONG: {
					jlong gotVal = java_impl->env->GetStaticLongField(clscls, fID);
					return value_create_long((long)gotVal);
				}

				case TYPE_FLOAT: {
					jfloat gotVal = java_impl->env->GetStaticFloatField(clscls, fID);
					return value_create_float((float)gotVal);
				}

				case TYPE_DOUBLE: {
					jdouble gotVal = java_impl->env->GetStaticDoubleField(clscls, fID);
					return value_create_double((double)gotVal);
				}

				case TYPE_STRING: {
					jstring gotVal = (jstring)java_impl->env->GetStaticObjectField(clscls, fID);
					const char *gotValConv = java_impl->env->GetStringUTFChars(gotVal, NULL);
					return value_create_string(gotValConv, strlen(gotValConv));
				}

				default: {
					log_write("metacall", LOG_LEVEL_ERROR, "Failed to convert type %s from Java to MetaCall value", type_name(fieldType));
					return NULL;
				}
			}
		}
	}

	return NULL;
}

int java_class_interface_static_set(klass cls, class_impl impl, attribute attr, value v)
{
	(void)cls;

	const char *key = (const char *)attribute_name(attr);
	type fieldType = (type)attribute_type(attr);

	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);
	loader_impl_java java_impl = java_cls->impl;

	jobject clsObj = java_cls->cls;
	jclass clscls = java_cls->concls;

	if (clscls != nullptr)
	{
		const char *fType = (const char *)type_derived(fieldType);
		jfieldID fID = java_cls->impl->env->GetStaticFieldID(clscls, key, fType);

		if (fID != nullptr)
		{
			type_id id = type_index(fieldType);

			switch (id)
			{
				case TYPE_BOOL: {
					jboolean val = (jboolean)value_to_bool(v);
					java_impl->env->SetStaticBooleanField(clscls, fID, val);
					return 0;
				}

				case TYPE_CHAR: {
					jchar val = (jchar)value_to_char(v);
					java_impl->env->SetStaticCharField(clscls, fID, val);
					return 0;
				}

				case TYPE_SHORT: {
					jshort val = (jshort)value_to_short(v);
					java_impl->env->SetStaticShortField(clscls, fID, val);
					return 0;
				}

				case TYPE_INT: {
					jint val = (jint)value_to_int(v);
					java_impl->env->SetStaticIntField(clscls, fID, val);
					return 0;
				}

				case TYPE_LONG: {
					jlong val = (jlong)value_to_long(v);
					java_impl->env->SetStaticLongField(clscls, fID, val);
					return 0;
				}

				case TYPE_FLOAT: {
					jfloat val = (jfloat)value_to_float(v);
					java_impl->env->SetStaticFloatField(clscls, fID, val);
					return 0;
				}

				case TYPE_DOUBLE: {
					jdouble val = (jdouble)value_to_double(v);
					java_impl->env->SetStaticDoubleField(clscls, fID, val);
					return 0;
				}

				case TYPE_STRING: {
					const char *strV = value_to_string(v);
					jstring val = java_impl->env->NewStringUTF(strV);
					java_impl->env->SetStaticObjectField(clscls, fID, val);
					return 0;
				}

				default: {
					log_write("metacall", LOG_LEVEL_ERROR, "Failed to convert type %s from MetaCall value to Java", type_name(fieldType));
					return NULL;
				}
			}
		}
	}

	return 1;
}

value java_class_interface_static_invoke(klass cls, class_impl impl, method m, class_args args, size_t argc)
{
	(void)cls;
	std::cout << "invoke" << std::endl;

	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);
	loader_impl_java java_impl = java_cls->impl;
	jclass clscls = java_cls->concls;

	loader_impl_java_method java_method = (loader_impl_java_method)method_data(m);
	const char *methodSignature = java_method->methodSignature;

	char *methodName = (char *)method_name(m);

	signature sg = method_signature(m);
	type t = signature_get_return(sg);

	jvalue *constructorArgs = nullptr;
	if (argc > 0)
	{
		constructorArgs = new jvalue[argc];
		getJValArray(constructorArgs, args, argc, java_cls->impl->env); // Create a jvalue array that can be passed to JNI
	}

	jmethodID function_invoke_id = java_impl->env->GetStaticMethodID(clscls, methodName, methodSignature);

	if (function_invoke_id != nullptr)
	{
		switch (type_index(t))
		{
			case TYPE_NULL: {
				java_impl->env->CallStaticVoidMethodA(clscls, function_invoke_id, constructorArgs);
				return value_create_null();
			}
			
			case TYPE_BOOL: {
				jboolean returnVal = (jboolean)java_impl->env->CallStaticBooleanMethodA(clscls, function_invoke_id, constructorArgs);
				return value_create_bool(returnVal);
			}

			case TYPE_CHAR: {
				jchar returnVal = (jchar)java_impl->env->CallStaticCharMethodA(clscls, function_invoke_id, constructorArgs);
				return value_create_char(returnVal);
			}

			case TYPE_SHORT: {
				jshort returnVal = (jshort)java_impl->env->CallStaticShortMethodA(clscls, function_invoke_id, constructorArgs);
				return value_create_short(returnVal);
			}

			case TYPE_INT: {
				jint returnVal = (jint)java_impl->env->CallStaticIntMethodA(clscls, function_invoke_id, constructorArgs);
				return value_create_int(returnVal);
			}

			case TYPE_LONG: {
				jlong returnVal = (jlong)java_impl->env->CallStaticLongMethodA(clscls, function_invoke_id, constructorArgs);
				return value_create_long(returnVal);
			}

			case TYPE_FLOAT: {
				jfloat returnVal = (jfloat)java_impl->env->CallStaticFloatMethodA(clscls, function_invoke_id, constructorArgs);
				return value_create_float(returnVal);
			}

			case TYPE_DOUBLE: {
				jdouble returnVal = (jdouble)java_impl->env->CallStaticDoubleMethodA(clscls, function_invoke_id, constructorArgs);
				return value_create_double(returnVal);
			}

			case TYPE_STRING: {
				jstring returnVal = (jstring)java_impl->env->CallStaticObjectMethodA(clscls, function_invoke_id, constructorArgs);
				const char *returnString = java_impl->env->GetStringUTFChars(returnVal, NULL);
				return value_create_string(returnString, strlen(returnString));
			}
		}
	}

	return NULL;
}

value java_class_interface_static_await(klass cls, class_impl impl, method m, class_args args, size_t size, class_resolve_callback resolve, class_reject_callback reject, void *ctx)
{
	// TODO
	(void)cls;
	(void)impl;
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
	(void)impl;
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

	const char *loader_library_path = value_to_string(configuration_value(config, "loader_library_path"));

	if (java_impl != nullptr)
	{
		std::string st = (std::string) "-Djava.class.path=" + loader_library_path;
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

		static struct
		{
			type_id id;
			const char *name;
			const char *jni_signature;
		} type_id_name_sig[] = {
			{ TYPE_NULL, "void", "V" },
			{ TYPE_BOOL, "boolean", "Z" },
			{ TYPE_CHAR, "char", "C" },
			{ TYPE_SHORT, "short", "S" },
			{ TYPE_INT, "int", "I" },
			{ TYPE_LONG, "long", "J" },
			{ TYPE_FLOAT, "float", "F" },
			{ TYPE_DOUBLE, "double", "D" },
			{ TYPE_STRING, "java.lang.String", "Ljava/lang/String;" },
		};

		size_t size = sizeof(type_id_name_sig) / sizeof(type_id_name_sig[0]);

		for (size_t i = 0; i < size; i++)
		{
			type t = type_create(type_id_name_sig[i].id, type_id_name_sig[i].name, (type_impl)type_id_name_sig[i].jni_signature, &type_java_singleton);

			if (!(t != NULL && loader_impl_type_define(impl, type_name(t), t) == 0))
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Failed to define type '%s' in Java Loader", type_id_name_sig[i].name);
			}
		}

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
		jobjectArray arr = java_impl->env->NewObjectArray((jsize)size, java_impl->env->FindClass("java/lang/String"), java_impl->env->NewStringUTF(""));

		for (size_t i = 0; i < size; i++) // Create JNI compatible array of paths
		{
			java_impl->env->SetObjectArrayElement(arr, (jsize)i, java_impl->env->NewStringUTF(paths[i]));
		}

		jclass classPtr = java_impl->env->FindClass("bootstrap");
		if (classPtr != nullptr)
		{
			jmethodID mid = java_impl->env->GetStaticMethodID(classPtr, "loadFromFile", "([Ljava/lang/String;)[Ljava/lang/Class;");
			if (mid != nullptr)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Function Found");

				jobjectArray result = (jobjectArray)java_impl->env->CallStaticObjectMethod(classPtr, mid, arr);

				java_handle->handle = result;
				java_handle->size = java_impl->env->GetArrayLength(result);

				java_impl->env->DeleteLocalRef(arr); // Remove the jObjectArray from memory

				// Check for errors
				if (java_handle->size != size)
				{
					java_impl->env->DeleteLocalRef(result);
					delete java_handle;
					return NULL;
				}

				return static_cast<loader_handle>(java_handle);
			}
		}
	}

	return NULL;
}

loader_handle java_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char *buffer, size_t size)
{
	(void)impl;

	loader_impl_java_handle java_handle = new loader_impl_java_handle_type();

	if (java_handle != nullptr)
	{
		loader_impl_java java_impl = static_cast<loader_impl_java>(loader_impl_get(impl));

		jclass classPtr = java_impl->env->FindClass("bootstrap");
		if (classPtr != nullptr)
		{
			jmethodID mid = java_impl->env->GetStaticMethodID(classPtr, "load_from_memory", "(Ljava/lang/String;Ljava/lang/String;)[Ljava/lang/Class;");
			if (mid != nullptr)
			{
				jobjectArray result = (jobjectArray)java_impl->env->CallStaticObjectMethod(classPtr, mid, java_impl->env->NewStringUTF(name), java_impl->env->NewStringUTF(buffer));

				java_handle->handle = result;
				java_handle->size = java_impl->env->GetArrayLength(result);

				return static_cast<loader_handle>(java_handle);
			}
		}
	}

	return NULL;
}

loader_handle java_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	(void)impl;
	(void)path;

	loader_impl_java_handle java_handle = new loader_impl_java_handle_type();

	if (java_handle != nullptr)
	{
		loader_impl_java java_impl = static_cast<loader_impl_java>(loader_impl_get(impl));

		jclass classPtr = java_impl->env->FindClass("bootstrap");
		if (classPtr != nullptr)
		{
			jmethodID mid = java_impl->env->GetStaticMethodID(classPtr, "load_from_package", "(Ljava/lang/String;)[Ljava/lang/Class;");
			if (mid != nullptr)
			{
				jobjectArray result = (jobjectArray)java_impl->env->CallStaticObjectMethod(classPtr, mid, java_impl->env->NewStringUTF(path));

				java_handle->handle = result;
				java_handle->size = java_impl->env->GetArrayLength(result);

				return static_cast<loader_handle>(java_handle);
			}
		}
	}

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

class_visibility_id getFieldVisibility(const char *v)
{
	if (!strcmp(v, "public"))
		return VISIBILITY_PUBLIC;
	if (!strcmp(v, "private"))
		return VISIBILITY_PRIVATE;
	if (!strcmp(v, "protected"))
		return VISIBILITY_PROTECTED;

	return VISIBILITY_PUBLIC; // Public by default
}

int java_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
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
			jsize handleSize = java_impl->env->GetArrayLength(java_handle->handle);

			if (handleSize == 0)
			{
				std::cout << "saved it" << std::endl;
				return 1;
			}

			for (jsize handle_index = 0; handle_index < handleSize; ++handle_index)
			{
				jobject r = java_impl->env->GetObjectArrayElement(java_handle->handle, handle_index);

				if (r != nullptr)
				{
					jstring result = (jstring)java_impl->env->CallStaticObjectMethod(classPtr, cls_name_bootstrap, r);
					const char *cls_name = java_impl->env->GetStringUTFChars(result, NULL);

					loader_impl_java_class java_cls = new loader_impl_java_class_type();

					java_cls->name = cls_name;
					java_cls->cls = r;
					java_cls->impl = (loader_impl_java)java_impl;

					klass c = class_create(cls_name, java_cls, &java_class_interface_singleton);

					jmethodID cls_field_array = java_impl->env->GetStaticMethodID(classPtr, "java_bootstrap_discover_fields", "(Ljava/lang/Class;)[Ljava/lang/reflect/Field;");
					if (cls_field_array != nullptr)
					{
						jobjectArray fieldArray = (jobjectArray)java_impl->env->CallStaticObjectMethod(classPtr, cls_field_array, r);
						jsize fieldArraySize = java_impl->env->GetArrayLength(fieldArray);

						jmethodID cls_field_details = java_impl->env->GetStaticMethodID(classPtr, "java_bootstrap_discover_fields_details", "(Ljava/lang/reflect/Field;)[Ljava/lang/String;");

						for (jsize field_index = 0; field_index < fieldArraySize; ++field_index)
						{
							jobject curField = java_impl->env->GetObjectArrayElement(fieldArray, field_index);
							jobjectArray fieldDetails = (jobjectArray)java_impl->env->CallStaticObjectMethod(classPtr, cls_field_details, curField);

							jstring fname = (jstring)java_impl->env->GetObjectArrayElement(fieldDetails, 0);
							const char *field_name = java_impl->env->GetStringUTFChars(fname, NULL);

							jstring ftype = (jstring)java_impl->env->GetObjectArrayElement(fieldDetails, 1);
							const char *field_type = java_impl->env->GetStringUTFChars(ftype, NULL);

							jstring fvisibility = (jstring)java_impl->env->GetObjectArrayElement(fieldDetails, 2);
							const char *field_visibility = java_impl->env->GetStringUTFChars(fvisibility, NULL);

							jstring fstatic = (jstring)java_impl->env->GetObjectArrayElement(fieldDetails, 3);
							const char *field_static = java_impl->env->GetStringUTFChars(fstatic, NULL);

							std::cout << field_visibility << " " << field_static << " " << field_type << " " << field_name << std::endl;

							loader_impl_java_field java_field = new loader_impl_java_field_type();
							java_field->fieldName = field_name;
							java_field->fieldObj = curField;

							type t = loader_impl_type(impl, field_type);

							// TODO: We should check if the type t is not known here (t == NULL), and if it is a new class
							// that has not been registered yet, we should register it dynamically using
							// loader_impl_type_define, and inspecting that class so we know the type

							attribute attr = attribute_create(c, field_name, t, java_field, getFieldVisibility(field_visibility));

							if (!strcmp(field_static, "static"))
								class_register_static_attribute(c, attr);
							else
								class_register_attribute(c, attr);
						}
					}

					jmethodID cls_method_array = java_impl->env->GetStaticMethodID(classPtr, "java_bootstrap_discover_methods", "(Ljava/lang/Class;)[Ljava/lang/reflect/Method;");
					if (cls_method_array != nullptr)
					{
						jobjectArray methodArray = (jobjectArray)java_impl->env->CallStaticObjectMethod(classPtr, cls_method_array, r);
						jsize methodArraySize = java_impl->env->GetArrayLength(methodArray);

						jmethodID cls_method_details = java_impl->env->GetStaticMethodID(classPtr, "java_bootstrap_discover_method_details", "(Ljava/lang/reflect/Method;)[Ljava/lang/String;");

						for (jsize method_index = 0; method_index < methodArraySize; ++method_index)
						{
							jobject curMethod = java_impl->env->GetObjectArrayElement(methodArray, method_index);
							jobjectArray methodDetails = (jobjectArray)java_impl->env->CallStaticObjectMethod(classPtr, cls_method_details, curMethod);

							jstring mname = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 0);
							const char *method_name = java_impl->env->GetStringUTFChars(mname, NULL);

							jstring mReturnType = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 1);
							const char *method_return_type = java_impl->env->GetStringUTFChars(mReturnType, NULL);

							jstring mVisibility = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 2);
							const char *method_visibility = java_impl->env->GetStringUTFChars(mVisibility, NULL);

							jstring mStatic = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 3);
							const char *method_static = java_impl->env->GetStringUTFChars(mStatic, NULL);

							jstring mSignature = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 4);
							const char *method_sig = java_impl->env->GetStringUTFChars(mSignature, NULL);

							jmethodID cls_method_args_size = java_impl->env->GetStaticMethodID(classPtr, "java_bootstrap_discover_method_args_size", "(Ljava/lang/reflect/Method;)I");
							jint args_count = (jint)java_impl->env->CallStaticIntMethod(classPtr, cls_method_args_size, curMethod);

							loader_impl_java_method java_method = new loader_impl_java_method_type();
							java_method->methodObj = curMethod;
							java_method->methodSignature = method_sig;

							// CREATING A NEW METHOD
							method m = method_create(c, method_name, (size_t)args_count, java_method, getFieldVisibility(method_visibility), SYNCHRONOUS);

							// REGISTERING THE METHOD PARAMETER WITH INDEX
							signature s = method_signature(m);

							jmethodID cls_method_parameter_list = java_impl->env->GetStaticMethodID(classPtr, "java_bootstrap_discover_method_parameters", "(Ljava/lang/reflect/Method;)[Ljava/lang/String;");
							jobjectArray methodParameterList = (jobjectArray)java_impl->env->CallStaticObjectMethod(classPtr, cls_method_parameter_list, curMethod);

							if (methodParameterList)
							{
								jsize parameterLength = java_impl->env->GetArrayLength(methodParameterList);

								for (jsize pIndex = 0; pIndex < parameterLength; pIndex++)
								{
									jstring cparameter = (jstring)java_impl->env->GetObjectArrayElement(methodParameterList, pIndex);
									const char *parameter = java_impl->env->GetStringUTFChars(cparameter, NULL);

									type pt = loader_impl_type(impl, parameter);
									if (pt != NULL)
									{
										signature_set(s, (size_t)pIndex, parameter, pt);
									}
								}
							}

							// REGISTERING THE METHOD RETURN PARAMETER
							type rt = loader_impl_type(impl, method_return_type);
							signature_set_return(s, rt);

							// METHOD REGISTERATION
							if (!strcmp(method_static, "static"))
								class_register_static_method(c, m);
							else
								class_register_method(c, m);
						}
					}

					scope sp = context_scope(ctx);
					scope_define(sp, cls_name, value_create_class(c));

					std::cout << cls_name << " Class Registered!\n"
							  << std::endl;
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
			std::cout << "JNI ERROR" << std::endl;
		}

		/* Destroy children loaders */
		loader_unload_children(impl);

		java_impl->jvm->DestroyJavaVM();

		delete java_impl;

		return 0;
	}

	return 1;
}
