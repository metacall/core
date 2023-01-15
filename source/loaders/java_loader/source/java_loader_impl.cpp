/*
 *	Loader Library by Parra Studios
 *	A plugin for loading java code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <algorithm>
#include <cstring>
#include <string>

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
	loader_impl impl;
	loader_impl_java java_impl;
} * loader_impl_java_class;

typedef struct loader_impl_java_object_type
{
	const char *name;
	jobject conObj;
	jclass concls;
	loader_impl_java java_impl;
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

static type_interface type_java_singleton(void);

static type java_loader_impl_type(loader_impl impl, const char *type_str, const char *type_signature)
{
	type t = loader_impl_type(impl, type_str);

	if (t != NULL)
	{
		return t;
	}

	type_id id;

	if (type_str[0] == '[')
	{
		id = TYPE_ARRAY;
	}
	else if (type_str[0] == 'L')
	{
		id = TYPE_OBJECT;
	}
	else
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Type %s is an unknown type, go to 'java_loader_impl_type' and implement it", type_str);
		return NULL;
	}

	t = type_create(id, type_str, new std::string(type_signature), &type_java_singleton);

	if (t == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid type allocation when discovering the type %s", type_str);
		return NULL;
	}

	if (loader_impl_type_define(impl, type_name(t), t) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Type %s could not be registered", type_str);
		type_destroy(t);
		return NULL;
	}

	return t;
}

static std::string array_get_subtype(const char *array_signature)
{
	size_t dimensions = 0;

	// Count number of first N occurrences of '['
	while (array_signature[dimensions++] == '[')
		;

	std::string subtype(&array_signature[dimensions]);

	subtype.pop_back();										// Remove ';
	std::replace(subtype.begin(), subtype.end(), '/', '.'); // Replace '/' by '.'

	return subtype;
}

static void getJNISignatureArgs(std::string &sig, class_args args, size_t argc)
{
	// TODO: I think that inspected constructors must be done, and we should avoid this
	// Maybe this does not fit properly with dynamic languages but at least, we must
	// allow this to be optional for strong typed languages

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

		if (id == TYPE_ARRAY)
		{
			sig += "[";

			// Only arrays of same type of elements are supported
			void *v = args[i];
			size_t size = value_type_count(v);

			if (size == 0)
			{
				// TODO: This will be deleted once we implement constructors, because we won't need to inspect
				// on the fly based on the caller arguments
				log_write("metacall", LOG_LEVEL_ERROR, "Arrays of size 0 are not supported in constructors");
				sig += "Ljava/lang/Object;"; // Using this as workaround
				continue;
			}

			void **arr = value_to_array(v);
			type_id arr_id = value_type_id(arr[0]);

			for (size_t arr_it = 1; arr_it < size; ++arr_it)
			{
				if (arr_id != value_type_id(arr[arr_it]))
				{
					log_write("metacall", LOG_LEVEL_ERROR,
						"Trying to pass an array of different subtypes to a constructor, this is not supported by Java, "
						"the element %" PRIuS " of the array is of type %" PRIuS " meanwhile the first element "
						"is of type %" PRIuS,
						arr_it, value_type_id(arr[arr_it]), arr_id);
				}
			}

			getJNISignatureArgs(sig, &arr[0], 1);
		}

		if (id == TYPE_OBJECT)
			sig += "Ljava/lang/Object;";

		if (id == TYPE_CLASS)
			sig += "Ljava/lang/Class;";
	}
}

static std::string getJNISignature(class_args args, size_t argc, const char *returnType)
{
	// TODO: I think that inspected constructors must be done, and we should avoid this
	// Maybe this does not fit properly with dynamic languages but at least, we must
	// allow this to be optional for strong typed languages
	std::string sig = "(";

	getJNISignatureArgs(sig, args, argc);

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
		}
		else if (id == TYPE_CHAR)
		{
			constructorArgs[i].c = value_to_char(args[i]);
		}
		else if (id == TYPE_SHORT)
		{
			constructorArgs[i].s = value_to_short(args[i]);
		}
		else if (id == TYPE_INT)
		{
			constructorArgs[i].i = value_to_int(args[i]);
		}
		else if (id == TYPE_LONG)
		{
			constructorArgs[i].j = value_to_long(args[i]);
		}
		else if (id == TYPE_FLOAT)
		{
			constructorArgs[i].f = value_to_float(args[i]);
		}
		else if (id == TYPE_DOUBLE)
		{
			constructorArgs[i].d = value_to_double(args[i]);
		}
		else if (id == TYPE_STRING)
		{
			const char *strV = value_to_string(args[i]);
			jstring str = env->NewStringUTF(strV);
			constructorArgs[i].l = str;
		}
		else if (id == TYPE_ARRAY)
		{
			value *array_value = value_to_array(args[i]);
			size_t array_size = (size_t)value_type_count(args[i]);

			if (array_size == 0)
			{
				jobjectArray arr = env->NewObjectArray((jsize)0, env->FindClass("java/lang/String"), env->NewStringUTF(""));
				constructorArgs[i].l = arr;
				return;
			}

			type_id tt = value_type_id(array_value[0]);
			switch (tt)
			{
				case TYPE_BOOL: {
					jbooleanArray setArr = env->NewBooleanArray((jsize)array_size);

					jboolean *fill = (jboolean *)malloc(array_size * sizeof(jboolean));
					for (size_t i = 0; i < array_size; i++)
						fill[i] = (jboolean)value_to_bool(array_value[i]);

					env->SetBooleanArrayRegion(setArr, 0, array_size, fill);
					constructorArgs[i].l = setArr;
					break;
				}
				case TYPE_CHAR: {
					jcharArray setArr = env->NewCharArray((jsize)array_size);

					jchar *fill = (jchar *)malloc(array_size * sizeof(jchar));
					for (size_t i = 0; i < array_size; i++)
						fill[i] = (jchar)value_to_char(array_value[i]);

					env->SetCharArrayRegion(setArr, 0, array_size, fill);
					constructorArgs[i].l = setArr;
					break;
				}
				case TYPE_SHORT: {
					jshortArray setArr = env->NewShortArray((jsize)array_size);

					jshort *fill = (jshort *)malloc(array_size * sizeof(jshort));
					for (size_t i = 0; i < array_size; i++)
						fill[i] = (jshort)value_to_short(array_value[i]);

					env->SetShortArrayRegion(setArr, 0, array_size, fill);
					constructorArgs[i].l = setArr;
					break;
				}

				case TYPE_INT: {
					jintArray setArr = env->NewIntArray((jsize)array_size);

					jint *fill = (jint *)malloc(array_size * sizeof(jint));

					for (size_t i = 0; i < array_size; i++)
						fill[i] = (jint)value_to_int(array_value[i]);

					env->SetIntArrayRegion(setArr, 0, array_size, fill);
					constructorArgs[i].l = setArr;
					break;
				}
				case TYPE_LONG: {
					jlongArray setArr = env->NewLongArray((jsize)array_size);

					jlong *fill = (jlong *)malloc(array_size * sizeof(jlong));

					for (size_t i = 0; i < array_size; i++)
						fill[i] = (jlong)value_to_long(array_value[i]);

					env->SetLongArrayRegion(setArr, 0, array_size, fill);
					constructorArgs[i].l = setArr;
					break;
				}
				case TYPE_FLOAT: {
					jfloatArray setArr = env->NewFloatArray((jsize)array_size);

					jfloat *fill = (jfloat *)malloc(array_size * sizeof(jfloat));

					for (size_t i = 0; i < array_size; i++)
						fill[i] = (jfloat)value_to_float(array_value[i]);

					env->SetFloatArrayRegion(setArr, 0, array_size, fill);
					constructorArgs[i].l = setArr;
					break;
				}
				case TYPE_DOUBLE: {
					jdoubleArray setArr = env->NewDoubleArray((jsize)array_size);

					jdouble *fill = (jdouble *)malloc(array_size * sizeof(jdouble));
					for (size_t i = 0; i < array_size; i++)
						fill[i] = (jdouble)value_to_double(array_value[i]);

					env->SetDoubleArrayRegion(setArr, 0, array_size, fill);
					constructorArgs[i].l = setArr;
					break;
				}

				case TYPE_STRING: {
					jobjectArray arr = env->NewObjectArray((jsize)array_size, env->FindClass("java/lang/String"), env->NewStringUTF(""));

					for (size_t i = 0; i < array_size; i++)
						env->SetObjectArrayElement(arr, (jsize)i, env->NewStringUTF(value_to_string(array_value[i])));

					constructorArgs[i].l = arr;
					break;
				}

				default:
					break;
			}
		}
	}
}

int type_java_interface_create(type t, type_impl impl)
{
	(void)t;
	(void)impl;

	return 0;
}

void type_java_interface_destroy(type t, type_impl impl)
{
	std::string *sig = static_cast<std::string *>(impl);

	(void)t;

	if (sig != nullptr)
	{
		delete sig;
	}
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
	(void)impl;

	return 0;
}

value java_object_interface_get(object obj, object_impl impl, struct accessor_type *accessor)
{
	(void)obj;

	attribute attr = accessor->data.attr;
	const char *key = (const char *)attribute_name(attr);
	type fieldType = (type)attribute_type(attr);

	loader_impl_java_object java_obj = static_cast<loader_impl_java_object>(impl);
	loader_impl_java java_impl = java_obj->java_impl;

	jobject clsObj = java_obj->conObj;
	jclass clscls = java_obj->concls;

	if (clscls != nullptr)
	{
		const char *fType = static_cast<std::string *>(type_derived(fieldType))->c_str();
		jfieldID fID = java_impl->env->GetFieldID(clscls, key, fType);

		if (fID != nullptr)
		{
			type_id id = type_index(fieldType);

			switch (id)
			{
				case TYPE_BOOL: {
					jboolean gotVal = java_impl->env->GetBooleanField(clsObj, fID);
					return value_create_bool((boolean)gotVal);
				}

				case TYPE_CHAR: {
					jchar gotVal = java_impl->env->GetCharField(clsObj, fID);
					return value_create_char((char)gotVal);
				}

				case TYPE_SHORT: {
					jshort gotVal = java_impl->env->GetShortField(clsObj, fID);
					return value_create_short((short)gotVal);
				}

				case TYPE_INT: {
					jint gotVal = java_impl->env->GetIntField(clsObj, fID);
					return value_create_int((int)gotVal);
				}

				case TYPE_LONG: {
					jlong gotVal = java_impl->env->GetLongField(clsObj, fID);
					return value_create_long((long)gotVal);
				}

				case TYPE_FLOAT: {
					jfloat gotVal = java_impl->env->GetFloatField(clsObj, fID);
					return value_create_float((float)gotVal);
				}

				case TYPE_DOUBLE: {
					jdouble gotVal = java_impl->env->GetDoubleField(clsObj, fID);
					return value_create_double((double)gotVal);
				}

				case TYPE_STRING: {
					jstring gotVal = (jstring)java_impl->env->GetObjectField(clsObj, fID);
					const char *gotValConv = java_impl->env->GetStringUTFChars(gotVal, NULL);
					return value_create_string(gotValConv, strlen(gotValConv));
				}

				case TYPE_ARRAY: {
					if (!strcmp(fType, "[Z"))
					{
						jbooleanArray gotVal = (jbooleanArray)java_impl->env->GetObjectField(clsObj, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jboolean *body = java_impl->env->GetBooleanArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_bool(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[C"))
					{
						jcharArray gotVal = (jcharArray)java_impl->env->GetObjectField(clsObj, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jchar *body = java_impl->env->GetCharArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_char(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[S"))
					{
						jshortArray gotVal = (jshortArray)java_impl->env->GetObjectField(clsObj, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jshort *body = java_impl->env->GetShortArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_short(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[I"))
					{
						jintArray gotVal = (jintArray)java_impl->env->GetObjectField(clsObj, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jint *body = java_impl->env->GetIntArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_int(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[J"))
					{
						jlongArray gotVal = (jlongArray)java_impl->env->GetObjectField(clsObj, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jlong *body = java_impl->env->GetLongArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_long(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[F"))
					{
						jfloatArray gotVal = (jfloatArray)java_impl->env->GetObjectField(clsObj, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jfloat *body = java_impl->env->GetFloatArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_float(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[D"))
					{
						jdoubleArray gotVal = (jdoubleArray)java_impl->env->GetObjectField(clsObj, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jdouble *body = java_impl->env->GetDoubleArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_double(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[Ljava/lang/String;"))
					{
						// TODO: Make this generic and recursive for any kind of array
						jobjectArray gotVal = (jobjectArray)java_impl->env->GetObjectField(clsObj, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						for (size_t i = 0; i < array_size; i++)
						{
							jstring cur_ele = (jstring)java_impl->env->GetObjectArrayElement(gotVal, i);
							const char *cur_element = java_impl->env->GetStringUTFChars(cur_ele, NULL);
							array_value[i] = value_create_string(cur_element, strlen(cur_element));
						}

						return v;
					}
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

int java_object_interface_set(object obj, object_impl impl, struct accessor_type *accessor, value v)
{
	(void)obj;

	attribute attr = accessor->data.attr;
	const char *key = (const char *)attribute_name(attr);
	type fieldType = (type)attribute_type(attr);

	loader_impl_java_object java_obj = static_cast<loader_impl_java_object>(impl);
	loader_impl_java java_impl = java_obj->java_impl;

	jobject conObj = java_obj->conObj;
	jclass clscls = java_obj->concls;

	if (clscls != nullptr)
	{
		const char *fType = static_cast<std::string *>(type_derived(fieldType))->c_str();
		jfieldID fID = java_impl->env->GetFieldID(clscls, key, fType);

		if (fID != nullptr)
		{
			type_id id = type_index(fieldType);

			switch (id)
			{
				case TYPE_BOOL: {
					jboolean val = (jboolean)value_to_bool(v);
					java_impl->env->SetBooleanField(conObj, fID, val);
					return 0;
				}

				case TYPE_CHAR: {
					jchar val = (jchar)value_to_char(v);
					java_impl->env->SetCharField(conObj, fID, val);
					return 0;
				}

				case TYPE_SHORT: {
					jshort val = (jshort)value_to_short(v);
					java_impl->env->SetShortField(conObj, fID, val);
					return 0;
				}

				case TYPE_INT: {
					jint val = (jint)value_to_int(v);
					java_impl->env->SetIntField(conObj, fID, val);
					return 0;
				}

				case TYPE_LONG: {
					jlong val = (jlong)value_to_long(v);
					java_impl->env->SetLongField(conObj, fID, val);
					return 0;
				}

				case TYPE_FLOAT: {
					jfloat val = (jfloat)value_to_float(v);
					java_impl->env->SetFloatField(conObj, fID, val);
					return 0;
				}

				case TYPE_DOUBLE: {
					jdouble val = (jdouble)value_to_double(v);
					java_impl->env->SetDoubleField(conObj, fID, val);
					return 0;
				}

				case TYPE_STRING: {
					const char *strV = value_to_string(v);
					jstring val = java_impl->env->NewStringUTF(strV);
					java_impl->env->SetObjectField(conObj, fID, val);
					return 0;
				}

				case TYPE_ARRAY: {
					value *array_value = value_to_array(v);
					size_t array_size = (size_t)value_type_count(v);

					if (!strcmp(fType, "[Z"))
					{
						jbooleanArray setArr = java_impl->env->NewBooleanArray((jsize)array_size);

						jboolean *fill = (jboolean *)malloc(array_size * sizeof(jboolean));

						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jboolean)value_to_bool(array_value[i]);

						java_impl->env->SetBooleanArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetObjectField(conObj, fID, setArr);
					}
					else if (!strcmp(fType, "[C"))
					{
						jcharArray setArr = java_impl->env->NewCharArray((jsize)array_size);

						jchar *fill = (jchar *)malloc(array_size * sizeof(jchar));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jchar)value_to_char(array_value[i]);

						java_impl->env->SetCharArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetObjectField(conObj, fID, setArr);
					}
					else if (!strcmp(fType, "[S"))
					{
						jshortArray setArr = java_impl->env->NewShortArray((jsize)array_size);

						jshort *fill = (jshort *)malloc(array_size * sizeof(jshort));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jshort)value_to_short(array_value[i]);

						java_impl->env->SetShortArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetObjectField(conObj, fID, setArr);
					}
					else if (!strcmp(fType, "[I"))
					{
						jintArray setArr = java_impl->env->NewIntArray((jsize)array_size);

						jint *fill = (jint *)malloc(array_size * sizeof(jint));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jint)value_to_int(array_value[i]);

						java_impl->env->SetIntArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetObjectField(conObj, fID, setArr);
					}
					else if (!strcmp(fType, "[J"))
					{
						jlongArray setArr = java_impl->env->NewLongArray((jsize)array_size);

						jlong *fill = (jlong *)malloc(array_size * sizeof(jlong));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jlong)value_to_long(array_value[i]);

						java_impl->env->SetLongArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetObjectField(conObj, fID, setArr);
					}
					else if (!strcmp(fType, "[F"))
					{
						jfloatArray setArr = java_impl->env->NewFloatArray((jsize)array_size);

						jfloat *fill = (jfloat *)malloc(array_size * sizeof(jfloat));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jfloat)value_to_float(array_value[i]);

						java_impl->env->SetFloatArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetObjectField(conObj, fID, setArr);
					}
					else if (!strcmp(fType, "[D"))
					{
						jdoubleArray setArr = java_impl->env->NewDoubleArray((jsize)array_size);

						jdouble *fill = (jdouble *)malloc(array_size * sizeof(jdouble));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jdouble)value_to_double(array_value[i]);

						java_impl->env->SetDoubleArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetObjectField(conObj, fID, setArr);
					}
					else if (!strcmp(fType, "[Ljava/lang/String;"))
					{
						// TODO: This should be more generic and include other types of objects, not only string
						jobjectArray setArr = java_impl->env->NewObjectArray((jsize)array_size, java_impl->env->FindClass("java/lang/String"), java_impl->env->NewStringUTF(""));

						for (size_t i = 0; i < array_size; i++)
							java_impl->env->SetObjectArrayElement(setArr, (jsize)i, java_impl->env->NewStringUTF(value_to_string(array_value[i])));

						java_impl->env->SetObjectField(conObj, fID, setArr);
					}

					return 0;
				}

				default: {
					log_write("metacall", LOG_LEVEL_ERROR, "Failed to convert type %s from MetaCall value to Java", type_name(fieldType));
					return 1;
				}
			}
		}
	}

	return 1;
}

value java_object_interface_method_invoke(object obj, object_impl impl, method m, object_args args, size_t argc)
{
	(void)obj;

	loader_impl_java_object java_obj = static_cast<loader_impl_java_object>(impl);
	loader_impl_java java_impl = java_obj->java_impl;
	jobject clsObj = java_obj->conObj;
	jclass clscls = java_obj->concls;

	loader_impl_java_method java_method = (loader_impl_java_method)method_data(m);
	const char *methodSignature = java_method->methodSignature;

	char *methodName = (char *)method_name(m);

	signature sg = method_signature(m);
	type t = signature_get_return(sg);

	jvalue *constructorArgs = nullptr;
	if (argc > 0)
	{
		constructorArgs = new jvalue[argc];
		getJValArray(constructorArgs, args, argc, java_impl->env); // Create a jvalue array that can be passed to JNI
	}

	jmethodID function_invoke_id = java_impl->env->GetMethodID(clscls, methodName, methodSignature);

	if (function_invoke_id != nullptr)
	{
		switch (type_index(t))
		{
			case TYPE_NULL: {
				java_impl->env->CallVoidMethodA(clsObj, function_invoke_id, constructorArgs);
				return value_create_null();
			}

			case TYPE_BOOL: {
				jboolean returnVal = (jboolean)java_impl->env->CallBooleanMethodA(clsObj, function_invoke_id, constructorArgs);
				return value_create_bool(returnVal);
			}

			case TYPE_CHAR: {
				jchar returnVal = (jchar)java_impl->env->CallCharMethodA(clsObj, function_invoke_id, constructorArgs);
				return value_create_char(returnVal);
			}

			case TYPE_SHORT: {
				jshort returnVal = (jshort)java_impl->env->CallShortMethodA(clsObj, function_invoke_id, constructorArgs);
				return value_create_short(returnVal);
			}

			case TYPE_INT: {
				jint returnVal = (jint)java_impl->env->CallIntMethodA(clsObj, function_invoke_id, constructorArgs);
				return value_create_int(returnVal);
			}

			case TYPE_LONG: {
				jlong returnVal = (jlong)java_impl->env->CallLongMethodA(clsObj, function_invoke_id, constructorArgs);
				return value_create_long(returnVal);
			}

			case TYPE_FLOAT: {
				jfloat returnVal = (jfloat)java_impl->env->CallFloatMethodA(clsObj, function_invoke_id, constructorArgs);
				return value_create_float(returnVal);
			}

			case TYPE_DOUBLE: {
				jdouble returnVal = (jdouble)java_impl->env->CallDoubleMethodA(clsObj, function_invoke_id, constructorArgs);
				return value_create_double(returnVal);
			}

			case TYPE_STRING: {
				jstring returnVal = (jstring)java_impl->env->CallObjectMethodA(clsObj, function_invoke_id, constructorArgs);
				const char *returnString = java_impl->env->GetStringUTFChars(returnVal, NULL);
				return value_create_string(returnString, strlen(returnString));
			}
		}
	}

	return NULL;
}

value java_object_interface_method_await(object obj, object_impl impl, method m, object_args args, size_t size, object_resolve_callback resolve, object_reject_callback reject, void *ctx)
{
	// Java doesnt support await
	(void)obj;
	(void)impl;
	(void)m;
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
	loader_impl_java_object java_obj = static_cast<loader_impl_java_object>(impl);

	(void)obj;

	if (java_obj != nullptr)
		delete java_obj;
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
	(void)impl;

	return 0;
}

object java_class_interface_constructor(klass cls, class_impl impl, const char *name, constructor ctor, class_args args, size_t argc)
{
	(void)cls;
	(void)ctor;

	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);
	loader_impl_java_object java_obj = new loader_impl_java_object_type();

	object obj = object_create(name, ACCESSOR_TYPE_STATIC, java_obj, &java_object_interface_singleton, cls);

	if (obj == NULL)
		return NULL;

	java_obj->java_impl = java_cls->java_impl;

	jvalue *constructorArgs = nullptr;

	if (argc > 0)
	{
		constructorArgs = new jvalue[argc];
		getJValArray(constructorArgs, args, argc, java_cls->java_impl->env); // Create a jvalue array that can be passed to JNI
	}

	if (java_cls->java_impl->env != nullptr && java_cls->cls != nullptr)
	{
		jclass classPtr = java_cls->java_impl->env->FindClass("bootstrap");

		if (classPtr != nullptr)
		{
			jmethodID findCls = java_cls->java_impl->env->GetStaticMethodID(classPtr, "FindClass", "(Ljava/lang/String;)Ljava/lang/Class;");
			if (findCls != nullptr)
			{
				jclass clscls = (jclass)java_cls->java_impl->env->CallStaticObjectMethod(classPtr, findCls, java_cls->java_impl->env->NewStringUTF(name));
				if (clscls != nullptr)
				{
					std::string sig = getJNISignature(args, argc, "void");

					jmethodID constMID = java_cls->java_impl->env->GetMethodID(clscls, "<init>", sig.c_str());
					if (constMID != nullptr)
					{
						jobject newCls = java_cls->java_impl->env->NewObjectA(clscls, constMID, constructorArgs);
						if (newCls != nullptr)
						{
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

value java_class_interface_static_get(klass cls, class_impl impl, struct accessor_type *accessor)
{
	(void)cls;

	attribute attr = accessor->data.attr;
	const char *key = (const char *)attribute_name(attr);
	type fieldType = (type)attribute_type(attr);
	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);
	loader_impl_java java_impl = java_cls->java_impl;
	jclass clscls = java_cls->concls;

	if (clscls != nullptr)
	{
		const char *fType = static_cast<std::string *>(type_derived(fieldType))->c_str();
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

				case TYPE_OBJECT: {
					jobject gotVal = java_impl->env->GetStaticObjectField(clscls, fID);
					jclass cls = (jclass)java_impl->env->GetObjectClass(gotVal);
					jmethodID mid_getName = java_impl->env->GetMethodID(cls, "getName", "()Ljava/lang/String;");
					jstring name = (jstring)java_impl->env->CallObjectMethod(cls, mid_getName);
					const char *cls_name = java_impl->env->GetStringUTFChars(name, NULL);
					/* TODO */
					// object obj = object_create()
					return value_create_object(NULL /* obj */);
				}

				case TYPE_CLASS: {
					jobject gotVal = java_impl->env->GetStaticObjectField(clscls, fID);
					jclass cls = (jclass)java_impl->env->GetObjectClass(gotVal);
					jmethodID mid_getName = java_impl->env->GetMethodID(cls, "getName", "()Ljava/lang/String;");
					jstring name = (jstring)java_impl->env->CallObjectMethod(gotVal, mid_getName);
					const char *cls_name = java_impl->env->GetStringUTFChars(name, NULL);
					value cls_val = loader_impl_get_value(java_cls->impl, cls_name);
					return value_type_copy(cls_val);
				}

				case TYPE_ARRAY: {
					// TODO: Support N dimensional arrays

					if (!strcmp(fType, "[Z"))
					{
						jbooleanArray gotVal = (jbooleanArray)java_impl->env->GetStaticObjectField(clscls, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jboolean *body = java_impl->env->GetBooleanArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_bool(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[C"))
					{
						jcharArray gotVal = (jcharArray)java_impl->env->GetStaticObjectField(clscls, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jchar *body = java_impl->env->GetCharArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_char(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[S"))
					{
						jshortArray gotVal = (jshortArray)java_impl->env->GetStaticObjectField(clscls, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jshort *body = java_impl->env->GetShortArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_short(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[I"))
					{
						jintArray gotVal = (jintArray)java_impl->env->GetStaticObjectField(clscls, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jint *body = java_impl->env->GetIntArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_int(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[J"))
					{
						jlongArray gotVal = (jlongArray)java_impl->env->GetStaticObjectField(clscls, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jlong *body = java_impl->env->GetLongArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_long(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[F"))
					{
						jfloatArray gotVal = (jfloatArray)java_impl->env->GetStaticObjectField(clscls, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jfloat *body = java_impl->env->GetFloatArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_float(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[D"))
					{
						jdoubleArray gotVal = (jdoubleArray)java_impl->env->GetStaticObjectField(clscls, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jdouble *body = java_impl->env->GetDoubleArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_double(body[i]);

						return v;
					}
					else if (fType[0] == '[' && fType[1] == 'L')
					{
						jobjectArray gotVal = (jobjectArray)java_impl->env->GetStaticObjectField(clscls, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);
						std::string subtype_str = array_get_subtype(fType);
						type subtype = java_loader_impl_type(java_cls->impl, subtype_str.c_str(), fType);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						switch (type_index(subtype))
						{
							case TYPE_STRING: {
								for (size_t i = 0; i < array_size; i++)
								{
									jstring cur_ele = (jstring)java_impl->env->GetObjectArrayElement(gotVal, i);
									const char *cur_element = java_impl->env->GetStringUTFChars(cur_ele, NULL);
									array_value[i] = value_create_string(cur_element, strlen(cur_element));
								}

								break;
							}
							case TYPE_OBJECT: {
								for (size_t i = 0; i < array_size; i++)
								{
									jobject cur_ele = (jobject)java_impl->env->GetObjectArrayElement(gotVal, i);
									jclass cls = (jclass)java_impl->env->GetObjectClass(cur_ele);
									jmethodID mid_getName = java_impl->env->GetMethodID(cls, "getName", "()Ljava/lang/String;");
									jstring name = (jstring)java_impl->env->CallObjectMethod(cls, mid_getName);
									const char *cls_name = java_impl->env->GetStringUTFChars(name, NULL);
									/* TODO */
									// object obj = object_create()
									array_value[i] = value_create_object(NULL /* obj */);
								}

								break;
							}
							case TYPE_CLASS: {
								for (size_t i = 0; i < array_size; i++)
								{
									jobject cur_ele = java_impl->env->GetObjectArrayElement(gotVal, i);
									jclass cls = (jclass)java_impl->env->GetObjectClass(cur_ele);
									jmethodID mid_getName = java_impl->env->GetMethodID(cls, "getName", "()Ljava/lang/String;");
									jstring name = (jstring)java_impl->env->CallObjectMethod(cur_ele, mid_getName);
									const char *cls_name = java_impl->env->GetStringUTFChars(name, NULL);
									value cls_val = loader_impl_get_value(java_cls->impl, cls_name);
									array_value[i] = value_type_copy(cls_val);
								}

								break;
							}
						}

						return v;
					}
				}

				default: {
					log_write("metacall", LOG_LEVEL_ERROR, "Failed to convert type %s from Java to MetaCall value", type_name(fieldType));
					break;
				}
			}
		}
	}

	return NULL;
}

int java_class_interface_static_set(klass cls, class_impl impl, struct accessor_type *accessor, value v)
{
	(void)cls;

	attribute attr = accessor->data.attr;
	const char *key = (const char *)attribute_name(attr);
	type fieldType = (type)attribute_type(attr);
	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);
	loader_impl_java java_impl = java_cls->java_impl;
	jclass clscls = java_cls->concls;

	if (clscls != nullptr)
	{
		const char *fType = static_cast<std::string *>(type_derived(fieldType))->c_str();
		jfieldID fID = java_cls->java_impl->env->GetStaticFieldID(clscls, key, fType);

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

				case TYPE_ARRAY: {
					value *array_value = value_to_array(v);
					size_t array_size = (size_t)value_type_count(v);

					if (!strcmp(fType, "[Z"))
					{
						jbooleanArray setArr = java_impl->env->NewBooleanArray((jsize)array_size);

						jboolean *fill = (jboolean *)malloc(array_size * sizeof(jboolean));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jboolean)value_to_bool(array_value[i]);

						java_impl->env->SetBooleanArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetStaticObjectField(clscls, fID, setArr);
					}
					else if (!strcmp(fType, "[C"))
					{
						jcharArray setArr = java_impl->env->NewCharArray((jsize)array_size);

						jchar *fill = (jchar *)malloc(array_size * sizeof(jchar));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jchar)value_to_char(array_value[i]);

						java_impl->env->SetCharArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetStaticObjectField(clscls, fID, setArr);
					}
					else if (!strcmp(fType, "[S"))
					{
						jshortArray setArr = java_impl->env->NewShortArray((jsize)array_size);

						jshort *fill = (jshort *)malloc(array_size * sizeof(jshort));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jshort)value_to_short(array_value[i]);

						java_impl->env->SetShortArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetStaticObjectField(clscls, fID, setArr);
					}
					else if (!strcmp(fType, "[I"))
					{
						jintArray setArr = java_impl->env->NewIntArray((jsize)array_size);

						jint *fill = (jint *)malloc(array_size * sizeof(jint));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jint)value_to_int(array_value[i]);

						java_impl->env->SetIntArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetStaticObjectField(clscls, fID, setArr);
					}
					else if (!strcmp(fType, "[J"))
					{
						jlongArray setArr = java_impl->env->NewLongArray((jsize)array_size);

						jlong *fill = (jlong *)malloc(array_size * sizeof(jlong));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jlong)value_to_long(array_value[i]);

						java_impl->env->SetLongArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetStaticObjectField(clscls, fID, setArr);
					}
					else if (!strcmp(fType, "[F"))
					{
						jfloatArray setArr = java_impl->env->NewFloatArray((jsize)array_size);

						jfloat *fill = (jfloat *)malloc(array_size * sizeof(jfloat));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jfloat)value_to_float(array_value[i]);

						java_impl->env->SetFloatArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetStaticObjectField(clscls, fID, setArr);
					}
					else if (!strcmp(fType, "[D"))
					{
						jdoubleArray setArr = java_impl->env->NewDoubleArray((jsize)array_size);

						jdouble *fill = (jdouble *)malloc(array_size * sizeof(jdouble));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jdouble)value_to_double(array_value[i]);

						java_impl->env->SetDoubleArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetStaticObjectField(clscls, fID, setArr);
					}
					else if (!strcmp(fType, "[Ljava/lang/String;"))
					{
						// TODO: Implement this for any kind of object, make it recursive
						jobjectArray arr = java_impl->env->NewObjectArray((jsize)array_size, java_impl->env->FindClass("java/lang/String"), java_impl->env->NewStringUTF(""));

						for (size_t i = 0; i < array_size; i++)
							java_impl->env->SetObjectArrayElement(arr, (jsize)i, java_impl->env->NewStringUTF(value_to_string(array_value[i])));

						java_impl->env->SetStaticObjectField(clscls, fID, arr);
					}

					return 0;
				}

				default: {
					log_write("metacall", LOG_LEVEL_ERROR, "Failed to convert type %s from MetaCall value to Java", type_name(fieldType));
					return 1;
				}
			}
		}
	}

	return 1;
}

value java_class_interface_static_invoke(klass cls, class_impl impl, method m, class_args args, size_t argc)
{
	(void)cls;

	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);
	loader_impl_java java_impl = java_cls->java_impl;
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
		getJValArray(constructorArgs, args, argc, java_cls->java_impl->env); // Create a jvalue array that can be passed to JNI
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
	// Java doesnt support await so skip this
	(void)cls;
	(void)impl;
	(void)m;
	(void)args;
	(void)size;
	(void)resolve;
	(void)reject;
	(void)ctx;

	return NULL;
}

void java_class_interface_destroy(klass cls, class_impl impl)
{
	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);

	(void)cls;

	if (java_cls != nullptr)
		delete java_cls;
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
			{ TYPE_OBJECT, "java.lang.Object", "Ljava/lang/Object;" },
			{ TYPE_CLASS, "java.lang.Class", "Ljava/lang/Class;" }
		};

		size_t size = sizeof(type_id_name_sig) / sizeof(type_id_name_sig[0]);

		for (size_t i = 0; i < size; i++)
		{
			type t = type_create(type_id_name_sig[i].id, type_id_name_sig[i].name, (type_impl) new std::string(type_id_name_sig[i].jni_signature), &type_java_singleton);

			if (t != NULL && loader_impl_type_define(impl, type_name(t), t) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Failed to define type '%s' in Java Loader", type_id_name_sig[i].name);
				type_destroy(t);
			}
		}

		/* Register initialization */
		loader_initialization_register(impl);

		return static_cast<loader_impl_data>(java_impl);
	}

	return NULL;
}

int java_loader_impl_execution_path(loader_impl impl, const loader_path path)
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

loader_handle java_loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size)
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

loader_handle java_loader_impl_load_from_memory(loader_impl impl, const loader_name name, const char *buffer, size_t size)
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

loader_handle java_loader_impl_load_from_package(loader_impl impl, const loader_path path)
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

				if (result == NULL)
				{
					delete java_handle;
					return NULL;
				}

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
				log_write("metacall", LOG_LEVEL_ERROR, "Trying to discover a handle without any class");
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
					java_cls->impl = impl;
					java_cls->java_impl = java_impl;

					klass c = class_create(cls_name, ACCESSOR_TYPE_STATIC, java_cls, &java_class_interface_singleton);

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

							jstring fSignature = (jstring)java_impl->env->GetObjectArrayElement(fieldDetails, 4);
							const char *field_signature = java_impl->env->GetStringUTFChars(fSignature, NULL);

							loader_impl_java_field java_field = new loader_impl_java_field_type();
							java_field->fieldName = field_name;
							java_field->fieldObj = curField;

							type t = java_loader_impl_type(impl, field_type, field_signature);

							if (t != NULL)
							{
								attribute attr = attribute_create(c, field_name, t, java_field, getFieldVisibility(field_visibility), NULL);

								if (!strcmp(field_static, "static"))
									class_register_static_attribute(c, attr);
								else
									class_register_attribute(c, attr);
							}
							else
							{
								log_write("metacall", LOG_LEVEL_ERROR, "Attribute %s could not be discovered, the type is not supported", field_name);
							}
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

							jstring mName = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 0);
							const char *m_name = java_impl->env->GetStringUTFChars(mName, NULL);

							jstring mReturnType = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 1);
							const char *m_return_type = java_impl->env->GetStringUTFChars(mReturnType, NULL);

							jstring mReturnTypeSig = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 2);
							const char *m_return_type_sig = java_impl->env->GetStringUTFChars(mReturnTypeSig, NULL);

							jstring mVisibility = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 3);
							const char *m_visibility = java_impl->env->GetStringUTFChars(mVisibility, NULL);

							jstring mStatic = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 4);
							const char *m_static = java_impl->env->GetStringUTFChars(mStatic, NULL);

							jstring mSignature = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 5);
							const char *m_sig = java_impl->env->GetStringUTFChars(mSignature, NULL);

							jmethodID cls_method_args_size = java_impl->env->GetStaticMethodID(classPtr, "java_bootstrap_discover_method_args_size", "(Ljava/lang/reflect/Method;)I");
							jint args_count = (jint)java_impl->env->CallStaticIntMethod(classPtr, cls_method_args_size, curMethod);

							loader_impl_java_method java_method = new loader_impl_java_method_type();
							java_method->methodObj = curMethod;
							java_method->methodSignature = m_sig;

							// CREATING A NEW METHOD
							method m = method_create(c, m_name, (size_t)args_count, java_method, getFieldVisibility(m_visibility), SYNCHRONOUS, NULL);

							// REGISTERING THE METHOD PARAMETER WITH INDEX
							signature s = method_signature(m);

							jmethodID cls_method_parameter_list = java_impl->env->GetStaticMethodID(classPtr, "java_bootstrap_discover_method_parameters", "(Ljava/lang/reflect/Method;)[[Ljava/lang/String;");
							jobjectArray methodParameterList = (jobjectArray)java_impl->env->CallStaticObjectMethod(classPtr, cls_method_parameter_list, curMethod);

							if (methodParameterList)
							{
								jsize parameterLength = java_impl->env->GetArrayLength(methodParameterList);

								for (jsize pIndex = 0; pIndex < parameterLength; pIndex++)
								{
									jobjectArray cparameter = (jobjectArray)java_impl->env->GetObjectArrayElement(methodParameterList, pIndex);

									jstring pName = (jstring)java_impl->env->GetObjectArrayElement(cparameter, 0);
									const char *p_name = java_impl->env->GetStringUTFChars(pName, NULL);

									jstring pSig = (jstring)java_impl->env->GetObjectArrayElement(cparameter, 1);
									const char *p_sig = java_impl->env->GetStringUTFChars(pSig, NULL);

									type pt = java_loader_impl_type(impl, p_name, p_sig);

									if (pt != NULL)
									{
										// Parameter names cannot be inspected with Reflection in Java (https://stackoverflow.com/questions/2237803/can-i-obtain-method-parameter-name-using-java-reflection)
										signature_set(s, (size_t)pIndex, "", pt);
									}
									else
									{
										log_write("metacall", LOG_LEVEL_ERROR, "The parameter type %s in method %s could not be registered, the type is not supported", p_name, m_name);
									}
								}
							}

							// REGISTERING THE METHOD RETURN PARAMETER
							type rt = java_loader_impl_type(impl, m_return_type, m_return_type_sig);

							if (rt != NULL)
							{
								signature_set_return(s, rt);
							}
							else
							{
								log_write("metacall", LOG_LEVEL_ERROR, "The return type %s in method %s could not be registered, the type is not supported", m_return_type, m_name);
							}

							// METHOD REGISTERATION
							if (!strcmp(m_static, "static"))
								class_register_static_method(c, m);
							else
								class_register_method(c, m);
						}
					}

					jmethodID cls_constructor_array = java_impl->env->GetStaticMethodID(classPtr, "java_bootstrap_discover_constructors", "(Ljava/lang/Class;)[Ljava/lang/reflect/Constructor;");

					if (cls_constructor_array != nullptr)
					{
						// TODO: Implement constructors
					}

					scope sp = context_scope(ctx);
					value v = value_create_class(c);

					if (scope_define(sp, cls_name, v) != 0)
					{
						value_type_destroy(v);
						return 1;
					}
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
			log_write("metacall", LOG_LEVEL_ERROR, "JNI failed to attach to the current thread");
		}

		/* Destroy children loaders */
		loader_unload_children(impl);

		java_impl->jvm->DestroyJavaVM();

		delete java_impl;

		return 0;
	}

	return 1;
} /*
   *	Loader Library by Parra Studios
   *	A plugin for loading java code at run-time into a process.
   *
   *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <algorithm>
#include <cstring>
#include <string>

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
	loader_impl impl;
	loader_impl_java java_impl;
} * loader_impl_java_class;

typedef struct loader_impl_java_object_type
{
	const char *name;
	jobject conObj;
	jclass concls;
	loader_impl_java java_impl;
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

static type_interface type_java_singleton(void);

static type java_loader_impl_type(loader_impl impl, const char *type_str, const char *type_signature)
{
	type t = loader_impl_type(impl, type_str);

	if (t != NULL)
	{
		return t;
	}

	type_id id;

	if (type_str[0] == '[')
	{
		id = TYPE_ARRAY;
	}
	else if (type_str[0] == 'L')
	{
		id = TYPE_OBJECT;
	}
	else
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Type %s is an unknown type, go to 'java_loader_impl_type' and implement it", type_str);
		return NULL;
	}

	t = type_create(id, type_str, new std::string(type_signature), &type_java_singleton);

	if (t == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid type allocation when discovering the type %s", type_str);
		return NULL;
	}

	if (loader_impl_type_define(impl, type_name(t), t) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Type %s could not be registered", type_str);
		type_destroy(t);
		return NULL;
	}

	return t;
}

static std::string array_get_subtype(const char *array_signature)
{
	size_t dimensions = 0;

	// Count number of first N occurrences of '['
	while (array_signature[dimensions++] == '[')
		;

	std::string subtype(&array_signature[dimensions]);

	subtype.pop_back();										// Remove ';
	std::replace(subtype.begin(), subtype.end(), '/', '.'); // Replace '/' by '.'

	return subtype;
}

static void getJNISignatureArgs(std::string &sig, class_args args, size_t argc)
{
	// TODO: I think that inspected constructors must be done, and we should avoid this
	// Maybe this does not fit properly with dynamic languages but at least, we must
	// allow this to be optional for strong typed languages

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

		if (id == TYPE_ARRAY)
		{
			sig += "[";

			// Only arrays of same type of elements are supported
			void *v = args[i];
			size_t size = value_type_count(v);

			if (size == 0)
			{
				// TODO: This will be deleted once we implement constructors, because we won't need to inspect
				// on the fly based on the caller arguments
				log_write("metacall", LOG_LEVEL_ERROR, "Arrays of size 0 are not supported in constructors");
				sig += "Ljava/lang/Object;"; // Using this as workaround
				continue;
			}

			void **arr = value_to_array(v);
			type_id arr_id = value_type_id(arr[0]);

			for (size_t arr_it = 1; arr_it < size; ++arr_it)
			{
				if (arr_id != value_type_id(arr[arr_it]))
				{
					log_write("metacall", LOG_LEVEL_ERROR,
						"Trying to pass an array of different subtypes to a constructor, this is not supported by Java, "
						"the element %" PRIuS " of the array is of type %" PRIuS " meanwhile the first element "
						"is of type %" PRIuS,
						arr_it, value_type_id(arr[arr_it]), arr_id);
				}
			}

			getJNISignatureArgs(sig, &arr[0], 1);
		}

		if (id == TYPE_OBJECT)
			sig += "Ljava/lang/Object;";

		if (id == TYPE_CLASS)
			sig += "Ljava/lang/Class;";
	}
}

static std::string getJNISignature(class_args args, size_t argc, const char *returnType)
{
	// TODO: I think that inspected constructors must be done, and we should avoid this
	// Maybe this does not fit properly with dynamic languages but at least, we must
	// allow this to be optional for strong typed languages
	std::string sig = "(";

	getJNISignatureArgs(sig, args, argc);

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
		}
		else if (id == TYPE_CHAR)
		{
			constructorArgs[i].c = value_to_char(args[i]);
		}
		else if (id == TYPE_SHORT)
		{
			constructorArgs[i].s = value_to_short(args[i]);
		}
		else if (id == TYPE_INT)
		{
			constructorArgs[i].i = value_to_int(args[i]);
		}
		else if (id == TYPE_LONG)
		{
			constructorArgs[i].j = value_to_long(args[i]);
		}
		else if (id == TYPE_FLOAT)
		{
			constructorArgs[i].f = value_to_float(args[i]);
		}
		else if (id == TYPE_DOUBLE)
		{
			constructorArgs[i].d = value_to_double(args[i]);
		}
		else if (id == TYPE_STRING)
		{
			const char *strV = value_to_string(args[i]);
			jstring str = env->NewStringUTF(strV);
			constructorArgs[i].l = str;
		}
		else if (id == TYPE_ARRAY)
		{
			value *array_value = value_to_array(args[i]);
			size_t array_size = (size_t)value_type_count(args[i]);

			if (array_size == 0)
			{
				jobjectArray arr = env->NewObjectArray((jsize)0, env->FindClass("java/lang/String"), env->NewStringUTF(""));
				constructorArgs[i].l = arr;
				return;
			}

			type_id tt = value_type_id(array_value[0]);
			switch (tt)
			{
				case TYPE_BOOL: {
					jbooleanArray setArr = env->NewBooleanArray((jsize)array_size);

					jboolean *fill = (jboolean *)malloc(array_size * sizeof(jboolean));
					for (size_t i = 0; i < array_size; i++)
						fill[i] = (jboolean)value_to_bool(array_value[i]);

					env->SetBooleanArrayRegion(setArr, 0, array_size, fill);
					constructorArgs[i].l = setArr;
					break;
				}
				case TYPE_CHAR: {
					jcharArray setArr = env->NewCharArray((jsize)array_size);

					jchar *fill = (jchar *)malloc(array_size * sizeof(jchar));
					for (size_t i = 0; i < array_size; i++)
						fill[i] = (jchar)value_to_char(array_value[i]);

					env->SetCharArrayRegion(setArr, 0, array_size, fill);
					constructorArgs[i].l = setArr;
					break;
				}
				case TYPE_SHORT: {
					jshortArray setArr = env->NewShortArray((jsize)array_size);

					jshort *fill = (jshort *)malloc(array_size * sizeof(jshort));
					for (size_t i = 0; i < array_size; i++)
						fill[i] = (jshort)value_to_short(array_value[i]);

					env->SetShortArrayRegion(setArr, 0, array_size, fill);
					constructorArgs[i].l = setArr;
					break;
				}

				case TYPE_INT: {
					jintArray setArr = env->NewIntArray((jsize)array_size);

					jint *fill = (jint *)malloc(array_size * sizeof(jint));

					for (size_t i = 0; i < array_size; i++)
						fill[i] = (jint)value_to_int(array_value[i]);

					env->SetIntArrayRegion(setArr, 0, array_size, fill);
					constructorArgs[i].l = setArr;
					break;
				}
				case TYPE_LONG: {
					jlongArray setArr = env->NewLongArray((jsize)array_size);

					jlong *fill = (jlong *)malloc(array_size * sizeof(jlong));

					for (size_t i = 0; i < array_size; i++)
						fill[i] = (jlong)value_to_long(array_value[i]);

					env->SetLongArrayRegion(setArr, 0, array_size, fill);
					constructorArgs[i].l = setArr;
					break;
				}
				case TYPE_FLOAT: {
					jfloatArray setArr = env->NewFloatArray((jsize)array_size);

					jfloat *fill = (jfloat *)malloc(array_size * sizeof(jfloat));

					for (size_t i = 0; i < array_size; i++)
						fill[i] = (jfloat)value_to_float(array_value[i]);

					env->SetFloatArrayRegion(setArr, 0, array_size, fill);
					constructorArgs[i].l = setArr;
					break;
				}
				case TYPE_DOUBLE: {
					jdoubleArray setArr = env->NewDoubleArray((jsize)array_size);

					jdouble *fill = (jdouble *)malloc(array_size * sizeof(jdouble));
					for (size_t i = 0; i < array_size; i++)
						fill[i] = (jdouble)value_to_double(array_value[i]);

					env->SetDoubleArrayRegion(setArr, 0, array_size, fill);
					constructorArgs[i].l = setArr;
					break;
				}

				case TYPE_STRING: {
					jobjectArray arr = env->NewObjectArray((jsize)array_size, env->FindClass("java/lang/String"), env->NewStringUTF(""));

					for (size_t i = 0; i < array_size; i++)
						env->SetObjectArrayElement(arr, (jsize)i, env->NewStringUTF(value_to_string(array_value[i])));

					constructorArgs[i].l = arr;
					break;
				}

				default:
					break;
			}
		}
	}
}

int type_java_interface_create(type t, type_impl impl)
{
	(void)t;
	(void)impl;

	return 0;
}

void type_java_interface_destroy(type t, type_impl impl)
{
	std::string *sig = static_cast<std::string *>(impl);

	(void)t;

	if (sig != nullptr)
	{
		delete sig;
	}
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
	(void)impl;

	return 0;
}

value java_object_interface_get(object obj, object_impl impl, struct accessor_type *accessor)
{
	(void)obj;

	attribute attr = accessor->data.attr;
	const char *key = (const char *)attribute_name(attr);
	type fieldType = (type)attribute_type(attr);

	loader_impl_java_object java_obj = static_cast<loader_impl_java_object>(impl);
	loader_impl_java java_impl = java_obj->java_impl;

	jobject clsObj = java_obj->conObj;
	jclass clscls = java_obj->concls;

	if (clscls != nullptr)
	{
		const char *fType = static_cast<std::string *>(type_derived(fieldType))->c_str();
		jfieldID fID = java_impl->env->GetFieldID(clscls, key, fType);

		if (fID != nullptr)
		{
			type_id id = type_index(fieldType);

			switch (id)
			{
				case TYPE_BOOL: {
					jboolean gotVal = java_impl->env->GetBooleanField(clsObj, fID);
					return value_create_bool((boolean)gotVal);
				}

				case TYPE_CHAR: {
					jchar gotVal = java_impl->env->GetCharField(clsObj, fID);
					return value_create_char((char)gotVal);
				}

				case TYPE_SHORT: {
					jshort gotVal = java_impl->env->GetShortField(clsObj, fID);
					return value_create_short((short)gotVal);
				}

				case TYPE_INT: {
					jint gotVal = java_impl->env->GetIntField(clsObj, fID);
					return value_create_int((int)gotVal);
				}

				case TYPE_LONG: {
					jlong gotVal = java_impl->env->GetLongField(clsObj, fID);
					return value_create_long((long)gotVal);
				}

				case TYPE_FLOAT: {
					jfloat gotVal = java_impl->env->GetFloatField(clsObj, fID);
					return value_create_float((float)gotVal);
				}

				case TYPE_DOUBLE: {
					jdouble gotVal = java_impl->env->GetDoubleField(clsObj, fID);
					return value_create_double((double)gotVal);
				}

				case TYPE_STRING: {
					jstring gotVal = (jstring)java_impl->env->GetObjectField(clsObj, fID);
					const char *gotValConv = java_impl->env->GetStringUTFChars(gotVal, NULL);
					return value_create_string(gotValConv, strlen(gotValConv));
				}

				case TYPE_ARRAY: {
					if (!strcmp(fType, "[Z"))
					{
						jbooleanArray gotVal = (jbooleanArray)java_impl->env->GetObjectField(clsObj, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jboolean *body = java_impl->env->GetBooleanArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_bool(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[C"))
					{
						jcharArray gotVal = (jcharArray)java_impl->env->GetObjectField(clsObj, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jchar *body = java_impl->env->GetCharArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_char(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[S"))
					{
						jshortArray gotVal = (jshortArray)java_impl->env->GetObjectField(clsObj, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jshort *body = java_impl->env->GetShortArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_short(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[I"))
					{
						jintArray gotVal = (jintArray)java_impl->env->GetObjectField(clsObj, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jint *body = java_impl->env->GetIntArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_int(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[J"))
					{
						jlongArray gotVal = (jlongArray)java_impl->env->GetObjectField(clsObj, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jlong *body = java_impl->env->GetLongArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_long(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[F"))
					{
						jfloatArray gotVal = (jfloatArray)java_impl->env->GetObjectField(clsObj, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jfloat *body = java_impl->env->GetFloatArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_float(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[D"))
					{
						jdoubleArray gotVal = (jdoubleArray)java_impl->env->GetObjectField(clsObj, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jdouble *body = java_impl->env->GetDoubleArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_double(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[Ljava/lang/String;"))
					{
						// TODO: Make this generic and recursive for any kind of array
						jobjectArray gotVal = (jobjectArray)java_impl->env->GetObjectField(clsObj, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						for (size_t i = 0; i < array_size; i++)
						{
							jstring cur_ele = (jstring)java_impl->env->GetObjectArrayElement(gotVal, i);
							const char *cur_element = java_impl->env->GetStringUTFChars(cur_ele, NULL);
							array_value[i] = value_create_string(cur_element, strlen(cur_element));
						}

						return v;
					}
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

int java_object_interface_set(object obj, object_impl impl, struct accessor_type *accessor, value v)
{
	(void)obj;

	attribute attr = accessor->data.attr;
	const char *key = (const char *)attribute_name(attr);
	type fieldType = (type)attribute_type(attr);

	loader_impl_java_object java_obj = static_cast<loader_impl_java_object>(impl);
	loader_impl_java java_impl = java_obj->java_impl;

	jobject conObj = java_obj->conObj;
	jclass clscls = java_obj->concls;

	if (clscls != nullptr)
	{
		const char *fType = static_cast<std::string *>(type_derived(fieldType))->c_str();
		jfieldID fID = java_impl->env->GetFieldID(clscls, key, fType);

		if (fID != nullptr)
		{
			type_id id = type_index(fieldType);

			switch (id)
			{
				case TYPE_BOOL: {
					jboolean val = (jboolean)value_to_bool(v);
					java_impl->env->SetBooleanField(conObj, fID, val);
					return 0;
				}

				case TYPE_CHAR: {
					jchar val = (jchar)value_to_char(v);
					java_impl->env->SetCharField(conObj, fID, val);
					return 0;
				}

				case TYPE_SHORT: {
					jshort val = (jshort)value_to_short(v);
					java_impl->env->SetShortField(conObj, fID, val);
					return 0;
				}

				case TYPE_INT: {
					jint val = (jint)value_to_int(v);
					java_impl->env->SetIntField(conObj, fID, val);
					return 0;
				}

				case TYPE_LONG: {
					jlong val = (jlong)value_to_long(v);
					java_impl->env->SetLongField(conObj, fID, val);
					return 0;
				}

				case TYPE_FLOAT: {
					jfloat val = (jfloat)value_to_float(v);
					java_impl->env->SetFloatField(conObj, fID, val);
					return 0;
				}

				case TYPE_DOUBLE: {
					jdouble val = (jdouble)value_to_double(v);
					java_impl->env->SetDoubleField(conObj, fID, val);
					return 0;
				}

				case TYPE_STRING: {
					const char *strV = value_to_string(v);
					jstring val = java_impl->env->NewStringUTF(strV);
					java_impl->env->SetObjectField(conObj, fID, val);
					return 0;
				}

				case TYPE_ARRAY: {
					value *array_value = value_to_array(v);
					size_t array_size = (size_t)value_type_count(v);

					if (!strcmp(fType, "[Z"))
					{
						jbooleanArray setArr = java_impl->env->NewBooleanArray((jsize)array_size);

						jboolean *fill = (jboolean *)malloc(array_size * sizeof(jboolean));

						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jboolean)value_to_bool(array_value[i]);

						java_impl->env->SetBooleanArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetObjectField(conObj, fID, setArr);
					}
					else if (!strcmp(fType, "[C"))
					{
						jcharArray setArr = java_impl->env->NewCharArray((jsize)array_size);

						jchar *fill = (jchar *)malloc(array_size * sizeof(jchar));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jchar)value_to_char(array_value[i]);

						java_impl->env->SetCharArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetObjectField(conObj, fID, setArr);
					}
					else if (!strcmp(fType, "[S"))
					{
						jshortArray setArr = java_impl->env->NewShortArray((jsize)array_size);

						jshort *fill = (jshort *)malloc(array_size * sizeof(jshort));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jshort)value_to_short(array_value[i]);

						java_impl->env->SetShortArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetObjectField(conObj, fID, setArr);
					}
					else if (!strcmp(fType, "[I"))
					{
						jintArray setArr = java_impl->env->NewIntArray((jsize)array_size);

						jint *fill = (jint *)malloc(array_size * sizeof(jint));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jint)value_to_int(array_value[i]);

						java_impl->env->SetIntArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetObjectField(conObj, fID, setArr);
					}
					else if (!strcmp(fType, "[J"))
					{
						jlongArray setArr = java_impl->env->NewLongArray((jsize)array_size);

						jlong *fill = (jlong *)malloc(array_size * sizeof(jlong));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jlong)value_to_long(array_value[i]);

						java_impl->env->SetLongArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetObjectField(conObj, fID, setArr);
					}
					else if (!strcmp(fType, "[F"))
					{
						jfloatArray setArr = java_impl->env->NewFloatArray((jsize)array_size);

						jfloat *fill = (jfloat *)malloc(array_size * sizeof(jfloat));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jfloat)value_to_float(array_value[i]);

						java_impl->env->SetFloatArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetObjectField(conObj, fID, setArr);
					}
					else if (!strcmp(fType, "[D"))
					{
						jdoubleArray setArr = java_impl->env->NewDoubleArray((jsize)array_size);

						jdouble *fill = (jdouble *)malloc(array_size * sizeof(jdouble));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jdouble)value_to_double(array_value[i]);

						java_impl->env->SetDoubleArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetObjectField(conObj, fID, setArr);
					}
					else if (!strcmp(fType, "[Ljava/lang/String;"))
					{
						// TODO: This should be more generic and include other types of objects, not only string
						jobjectArray setArr = java_impl->env->NewObjectArray((jsize)array_size, java_impl->env->FindClass("java/lang/String"), java_impl->env->NewStringUTF(""));

						for (size_t i = 0; i < array_size; i++)
							java_impl->env->SetObjectArrayElement(setArr, (jsize)i, java_impl->env->NewStringUTF(value_to_string(array_value[i])));

						java_impl->env->SetObjectField(conObj, fID, setArr);
					}

					return 0;
				}

				default: {
					log_write("metacall", LOG_LEVEL_ERROR, "Failed to convert type %s from MetaCall value to Java", type_name(fieldType));
					return 1;
				}
			}
		}
	}

	return 1;
}

value java_object_interface_method_invoke(object obj, object_impl impl, method m, object_args args, size_t argc)
{
	(void)obj;

	loader_impl_java_object java_obj = static_cast<loader_impl_java_object>(impl);
	loader_impl_java java_impl = java_obj->java_impl;
	jobject clsObj = java_obj->conObj;
	jclass clscls = java_obj->concls;

	loader_impl_java_method java_method = (loader_impl_java_method)method_data(m);
	const char *methodSignature = java_method->methodSignature;

	char *methodName = (char *)method_name(m);

	signature sg = method_signature(m);
	type t = signature_get_return(sg);

	jvalue *constructorArgs = nullptr;
	if (argc > 0)
	{
		constructorArgs = new jvalue[argc];
		getJValArray(constructorArgs, args, argc, java_impl->env); // Create a jvalue array that can be passed to JNI
	}

	jmethodID function_invoke_id = java_impl->env->GetMethodID(clscls, methodName, methodSignature);

	if (function_invoke_id != nullptr)
	{
		switch (type_index(t))
		{
			case TYPE_NULL: {
				java_impl->env->CallVoidMethodA(clsObj, function_invoke_id, constructorArgs);
				return value_create_null();
			}

			case TYPE_BOOL: {
				jboolean returnVal = (jboolean)java_impl->env->CallBooleanMethodA(clsObj, function_invoke_id, constructorArgs);
				return value_create_bool(returnVal);
			}

			case TYPE_CHAR: {
				jchar returnVal = (jchar)java_impl->env->CallCharMethodA(clsObj, function_invoke_id, constructorArgs);
				return value_create_char(returnVal);
			}

			case TYPE_SHORT: {
				jshort returnVal = (jshort)java_impl->env->CallShortMethodA(clsObj, function_invoke_id, constructorArgs);
				return value_create_short(returnVal);
			}

			case TYPE_INT: {
				jint returnVal = (jint)java_impl->env->CallIntMethodA(clsObj, function_invoke_id, constructorArgs);
				return value_create_int(returnVal);
			}

			case TYPE_LONG: {
				jlong returnVal = (jlong)java_impl->env->CallLongMethodA(clsObj, function_invoke_id, constructorArgs);
				return value_create_long(returnVal);
			}

			case TYPE_FLOAT: {
				jfloat returnVal = (jfloat)java_impl->env->CallFloatMethodA(clsObj, function_invoke_id, constructorArgs);
				return value_create_float(returnVal);
			}

			case TYPE_DOUBLE: {
				jdouble returnVal = (jdouble)java_impl->env->CallDoubleMethodA(clsObj, function_invoke_id, constructorArgs);
				return value_create_double(returnVal);
			}

			case TYPE_STRING: {
				jstring returnVal = (jstring)java_impl->env->CallObjectMethodA(clsObj, function_invoke_id, constructorArgs);
				const char *returnString = java_impl->env->GetStringUTFChars(returnVal, NULL);
				return value_create_string(returnString, strlen(returnString));
			}
		}
	}

	return NULL;
}

value java_object_interface_method_await(object obj, object_impl impl, method m, object_args args, size_t size, object_resolve_callback resolve, object_reject_callback reject, void *ctx)
{
	// Java doesnt support await
	(void)obj;
	(void)impl;
	(void)m;
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
	loader_impl_java_object java_obj = static_cast<loader_impl_java_object>(impl);

	(void)obj;

	if (java_obj != nullptr)
		delete java_obj;
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
	(void)impl;

	return 0;
}

object java_class_interface_constructor(klass cls, class_impl impl, const char *name, constructor ctor, class_args args, size_t argc)
{
	(void)cls;
	(void)ctor;

	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);
	loader_impl_java_object java_obj = new loader_impl_java_object_type();

	object obj = object_create(name, ACCESSOR_TYPE_STATIC, java_obj, &java_object_interface_singleton, cls);

	if (obj == NULL)
		return NULL;

	java_obj->java_impl = java_cls->java_impl;

	jvalue *constructorArgs = nullptr;

	if (argc > 0)
	{
		constructorArgs = new jvalue[argc];
		getJValArray(constructorArgs, args, argc, java_cls->java_impl->env); // Create a jvalue array that can be passed to JNI
	}

	if (java_cls->java_impl->env != nullptr && java_cls->cls != nullptr)
	{
		jclass classPtr = java_cls->java_impl->env->FindClass("bootstrap");

		if (classPtr != nullptr)
		{
			jmethodID findCls = java_cls->java_impl->env->GetStaticMethodID(classPtr, "FindClass", "(Ljava/lang/String;)Ljava/lang/Class;");
			if (findCls != nullptr)
			{
				jclass clscls = (jclass)java_cls->java_impl->env->CallStaticObjectMethod(classPtr, findCls, java_cls->java_impl->env->NewStringUTF(name));
				if (clscls != nullptr)
				{
					std::string sig = getJNISignature(args, argc, "void");

					jmethodID constMID = java_cls->java_impl->env->GetMethodID(clscls, "<init>", sig.c_str());
					if (constMID != nullptr)
					{
						jobject newCls = java_cls->java_impl->env->NewObjectA(clscls, constMID, constructorArgs);
						if (newCls != nullptr)
						{
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

value java_class_interface_static_get(klass cls, class_impl impl, struct accessor_type *accessor)
{
	(void)cls;

	attribute attr = accessor->data.attr;
	const char *key = (const char *)attribute_name(attr);
	type fieldType = (type)attribute_type(attr);
	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);
	loader_impl_java java_impl = java_cls->java_impl;
	jclass clscls = java_cls->concls;

	if (clscls != nullptr)
	{
		const char *fType = static_cast<std::string *>(type_derived(fieldType))->c_str();
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

				case TYPE_OBJECT: {
					jobject gotVal = java_impl->env->GetStaticObjectField(clscls, fID);
					jclass cls = (jclass)java_impl->env->GetObjectClass(gotVal);
					jmethodID mid_getName = java_impl->env->GetMethodID(cls, "getName", "()Ljava/lang/String;");
					jstring name = (jstring)java_impl->env->CallObjectMethod(cls, mid_getName);
					const char *cls_name = java_impl->env->GetStringUTFChars(name, NULL);
					/* TODO */
					// object obj = object_create()
					return value_create_object(NULL /* obj */);
				}

				case TYPE_CLASS: {
					jobject gotVal = java_impl->env->GetStaticObjectField(clscls, fID);
					jclass cls = (jclass)java_impl->env->GetObjectClass(gotVal);
					jmethodID mid_getName = java_impl->env->GetMethodID(cls, "getName", "()Ljava/lang/String;");
					jstring name = (jstring)java_impl->env->CallObjectMethod(gotVal, mid_getName);
					const char *cls_name = java_impl->env->GetStringUTFChars(name, NULL);
					value cls_val = loader_impl_get_value(java_cls->impl, cls_name);
					return value_type_copy(cls_val);
				}

				case TYPE_ARRAY: {
					// TODO: Support N dimensional arrays

					if (!strcmp(fType, "[Z"))
					{
						jbooleanArray gotVal = (jbooleanArray)java_impl->env->GetStaticObjectField(clscls, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jboolean *body = java_impl->env->GetBooleanArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_bool(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[C"))
					{
						jcharArray gotVal = (jcharArray)java_impl->env->GetStaticObjectField(clscls, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jchar *body = java_impl->env->GetCharArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_char(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[S"))
					{
						jshortArray gotVal = (jshortArray)java_impl->env->GetStaticObjectField(clscls, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jshort *body = java_impl->env->GetShortArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_short(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[I"))
					{
						jintArray gotVal = (jintArray)java_impl->env->GetStaticObjectField(clscls, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jint *body = java_impl->env->GetIntArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_int(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[J"))
					{
						jlongArray gotVal = (jlongArray)java_impl->env->GetStaticObjectField(clscls, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jlong *body = java_impl->env->GetLongArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_long(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[F"))
					{
						jfloatArray gotVal = (jfloatArray)java_impl->env->GetStaticObjectField(clscls, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jfloat *body = java_impl->env->GetFloatArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_float(body[i]);

						return v;
					}
					else if (!strcmp(fType, "[D"))
					{
						jdoubleArray gotVal = (jdoubleArray)java_impl->env->GetStaticObjectField(clscls, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						jdouble *body = java_impl->env->GetDoubleArrayElements(gotVal, 0);
						for (size_t i = 0; i < array_size; i++)
							array_value[i] = value_create_double(body[i]);

						return v;
					}
					else if (fType[0] == '[' && fType[1] == 'L')
					{
						jobjectArray gotVal = (jobjectArray)java_impl->env->GetStaticObjectField(clscls, fID);
						size_t array_size = (size_t)java_impl->env->GetArrayLength(gotVal);
						std::string subtype_str = array_get_subtype(fType);
						type subtype = java_loader_impl_type(java_cls->impl, subtype_str.c_str(), fType);

						void *v = value_create_array(NULL, (size_t)array_size);
						value *array_value = value_to_array(v);

						switch (type_index(subtype))
						{
							case TYPE_STRING: {
								for (size_t i = 0; i < array_size; i++)
								{
									jstring cur_ele = (jstring)java_impl->env->GetObjectArrayElement(gotVal, i);
									const char *cur_element = java_impl->env->GetStringUTFChars(cur_ele, NULL);
									array_value[i] = value_create_string(cur_element, strlen(cur_element));
								}

								break;
							}
							case TYPE_OBJECT: {
								for (size_t i = 0; i < array_size; i++)
								{
									jobject cur_ele = (jobject)java_impl->env->GetObjectArrayElement(gotVal, i);
									jclass cls = (jclass)java_impl->env->GetObjectClass(cur_ele);
									jmethodID mid_getName = java_impl->env->GetMethodID(cls, "getName", "()Ljava/lang/String;");
									jstring name = (jstring)java_impl->env->CallObjectMethod(cls, mid_getName);
									const char *cls_name = java_impl->env->GetStringUTFChars(name, NULL);
									/* TODO */
									// object obj = object_create()
									array_value[i] = value_create_object(NULL /* obj */);
								}

								break;
							}
							case TYPE_CLASS: {
								for (size_t i = 0; i < array_size; i++)
								{
									jobject cur_ele = java_impl->env->GetObjectArrayElement(gotVal, i);
									jclass cls = (jclass)java_impl->env->GetObjectClass(cur_ele);
									jmethodID mid_getName = java_impl->env->GetMethodID(cls, "getName", "()Ljava/lang/String;");
									jstring name = (jstring)java_impl->env->CallObjectMethod(cur_ele, mid_getName);
									const char *cls_name = java_impl->env->GetStringUTFChars(name, NULL);
									value cls_val = loader_impl_get_value(java_cls->impl, cls_name);
									array_value[i] = value_type_copy(cls_val);
								}

								break;
							}
						}

						return v;
					}
				}

				default: {
					log_write("metacall", LOG_LEVEL_ERROR, "Failed to convert type %s from Java to MetaCall value", type_name(fieldType));
					break;
				}
			}
		}
	}

	return NULL;
}

int java_class_interface_static_set(klass cls, class_impl impl, struct accessor_type *accessor, value v)
{
	(void)cls;

	attribute attr = accessor->data.attr;
	const char *key = (const char *)attribute_name(attr);
	type fieldType = (type)attribute_type(attr);
	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);
	loader_impl_java java_impl = java_cls->java_impl;
	jclass clscls = java_cls->concls;

	if (clscls != nullptr)
	{
		const char *fType = static_cast<std::string *>(type_derived(fieldType))->c_str();
		jfieldID fID = java_cls->java_impl->env->GetStaticFieldID(clscls, key, fType);

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

				case TYPE_ARRAY: {
					value *array_value = value_to_array(v);
					size_t array_size = (size_t)value_type_count(v);

					if (!strcmp(fType, "[Z"))
					{
						jbooleanArray setArr = java_impl->env->NewBooleanArray((jsize)array_size);

						jboolean *fill = (jboolean *)malloc(array_size * sizeof(jboolean));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jboolean)value_to_bool(array_value[i]);

						java_impl->env->SetBooleanArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetStaticObjectField(clscls, fID, setArr);
					}
					else if (!strcmp(fType, "[C"))
					{
						jcharArray setArr = java_impl->env->NewCharArray((jsize)array_size);

						jchar *fill = (jchar *)malloc(array_size * sizeof(jchar));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jchar)value_to_char(array_value[i]);

						java_impl->env->SetCharArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetStaticObjectField(clscls, fID, setArr);
					}
					else if (!strcmp(fType, "[S"))
					{
						jshortArray setArr = java_impl->env->NewShortArray((jsize)array_size);

						jshort *fill = (jshort *)malloc(array_size * sizeof(jshort));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jshort)value_to_short(array_value[i]);

						java_impl->env->SetShortArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetStaticObjectField(clscls, fID, setArr);
					}
					else if (!strcmp(fType, "[I"))
					{
						jintArray setArr = java_impl->env->NewIntArray((jsize)array_size);

						jint *fill = (jint *)malloc(array_size * sizeof(jint));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jint)value_to_int(array_value[i]);

						java_impl->env->SetIntArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetStaticObjectField(clscls, fID, setArr);
					}
					else if (!strcmp(fType, "[J"))
					{
						jlongArray setArr = java_impl->env->NewLongArray((jsize)array_size);

						jlong *fill = (jlong *)malloc(array_size * sizeof(jlong));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jlong)value_to_long(array_value[i]);

						java_impl->env->SetLongArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetStaticObjectField(clscls, fID, setArr);
					}
					else if (!strcmp(fType, "[F"))
					{
						jfloatArray setArr = java_impl->env->NewFloatArray((jsize)array_size);

						jfloat *fill = (jfloat *)malloc(array_size * sizeof(jfloat));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jfloat)value_to_float(array_value[i]);

						java_impl->env->SetFloatArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetStaticObjectField(clscls, fID, setArr);
					}
					else if (!strcmp(fType, "[D"))
					{
						jdoubleArray setArr = java_impl->env->NewDoubleArray((jsize)array_size);

						jdouble *fill = (jdouble *)malloc(array_size * sizeof(jdouble));
						for (size_t i = 0; i < array_size; i++)
							fill[i] = (jdouble)value_to_double(array_value[i]);

						java_impl->env->SetDoubleArrayRegion(setArr, 0, array_size, fill);
						java_impl->env->SetStaticObjectField(clscls, fID, setArr);
					}
					else if (!strcmp(fType, "[Ljava/lang/String;"))
					{
						// TODO: Implement this for any kind of object, make it recursive
						jobjectArray arr = java_impl->env->NewObjectArray((jsize)array_size, java_impl->env->FindClass("java/lang/String"), java_impl->env->NewStringUTF(""));

						for (size_t i = 0; i < array_size; i++)
							java_impl->env->SetObjectArrayElement(arr, (jsize)i, java_impl->env->NewStringUTF(value_to_string(array_value[i])));

						java_impl->env->SetStaticObjectField(clscls, fID, arr);
					}

					return 0;
				}

				default: {
					log_write("metacall", LOG_LEVEL_ERROR, "Failed to convert type %s from MetaCall value to Java", type_name(fieldType));
					return 1;
				}
			}
		}
	}

	return 1;
}

value java_class_interface_static_invoke(klass cls, class_impl impl, method m, class_args args, size_t argc)
{
	(void)cls;

	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);
	loader_impl_java java_impl = java_cls->java_impl;
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
		getJValArray(constructorArgs, args, argc, java_cls->java_impl->env); // Create a jvalue array that can be passed to JNI
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
	// Java doesnt support await so skip this
	(void)cls;
	(void)impl;
	(void)m;
	(void)args;
	(void)size;
	(void)resolve;
	(void)reject;
	(void)ctx;

	return NULL;
}

void java_class_interface_destroy(klass cls, class_impl impl)
{
	loader_impl_java_class java_cls = static_cast<loader_impl_java_class>(impl);

	(void)cls;

	if (java_cls != nullptr)
		delete java_cls;
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
			{ TYPE_OBJECT, "java.lang.Object", "Ljava/lang/Object;" },
			{ TYPE_CLASS, "java.lang.Class", "Ljava/lang/Class;" }
		};

		size_t size = sizeof(type_id_name_sig) / sizeof(type_id_name_sig[0]);

		for (size_t i = 0; i < size; i++)
		{
			type t = type_create(type_id_name_sig[i].id, type_id_name_sig[i].name, (type_impl) new std::string(type_id_name_sig[i].jni_signature), &type_java_singleton);

			if (t != NULL && loader_impl_type_define(impl, type_name(t), t) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Failed to define type '%s' in Java Loader", type_id_name_sig[i].name);
				type_destroy(t);
			}
		}

		/* Register initialization */
		loader_initialization_register(impl);

		return static_cast<loader_impl_data>(java_impl);
	}

	return NULL;
}

int java_loader_impl_execution_path(loader_impl impl, const loader_path path)
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

loader_handle java_loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size)
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

loader_handle java_loader_impl_load_from_memory(loader_impl impl, const loader_name name, const char *buffer, size_t size)
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

loader_handle java_loader_impl_load_from_package(loader_impl impl, const loader_path path)
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

				if (result == NULL)
				{
					delete java_handle;
					return NULL;
				}

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
				log_write("metacall", LOG_LEVEL_ERROR, "Trying to discover a handle without any class");
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
					java_cls->impl = impl;
					java_cls->java_impl = java_impl;

					klass c = class_create(cls_name, ACCESSOR_TYPE_STATIC, java_cls, &java_class_interface_singleton);

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

							jstring fSignature = (jstring)java_impl->env->GetObjectArrayElement(fieldDetails, 4);
							const char *field_signature = java_impl->env->GetStringUTFChars(fSignature, NULL);

							loader_impl_java_field java_field = new loader_impl_java_field_type();
							java_field->fieldName = field_name;
							java_field->fieldObj = curField;

							type t = java_loader_impl_type(impl, field_type, field_signature);

							if (t != NULL)
							{
								attribute attr = attribute_create(c, field_name, t, java_field, getFieldVisibility(field_visibility), NULL);

								if (!strcmp(field_static, "static"))
									class_register_static_attribute(c, attr);
								else
									class_register_attribute(c, attr);
							}
							else
							{
								log_write("metacall", LOG_LEVEL_ERROR, "Attribute %s could not be discovered, the type is not supported", field_name);
							}
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

							jstring mName = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 0);
							const char *m_name = java_impl->env->GetStringUTFChars(mName, NULL);

							jstring mReturnType = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 1);
							const char *m_return_type = java_impl->env->GetStringUTFChars(mReturnType, NULL);

							jstring mReturnTypeSig = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 2);
							const char *m_return_type_sig = java_impl->env->GetStringUTFChars(mReturnTypeSig, NULL);

							jstring mVisibility = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 3);
							const char *m_visibility = java_impl->env->GetStringUTFChars(mVisibility, NULL);

							jstring mStatic = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 4);
							const char *m_static = java_impl->env->GetStringUTFChars(mStatic, NULL);

							jstring mSignature = (jstring)java_impl->env->GetObjectArrayElement(methodDetails, 5);
							const char *m_sig = java_impl->env->GetStringUTFChars(mSignature, NULL);

							jmethodID cls_method_args_size = java_impl->env->GetStaticMethodID(classPtr, "java_bootstrap_discover_method_args_size", "(Ljava/lang/reflect/Method;)I");
							jint args_count = (jint)java_impl->env->CallStaticIntMethod(classPtr, cls_method_args_size, curMethod);

							loader_impl_java_method java_method = new loader_impl_java_method_type();
							java_method->methodObj = curMethod;
							java_method->methodSignature = m_sig;

							// CREATING A NEW METHOD
							method m = method_create(c, m_name, (size_t)args_count, java_method, getFieldVisibility(m_visibility), SYNCHRONOUS, NULL);

							// REGISTERING THE METHOD PARAMETER WITH INDEX
							signature s = method_signature(m);

							jmethodID cls_method_parameter_list = java_impl->env->GetStaticMethodID(classPtr, "java_bootstrap_discover_method_parameters", "(Ljava/lang/reflect/Method;)[[Ljava/lang/String;");
							jobjectArray methodParameterList = (jobjectArray)java_impl->env->CallStaticObjectMethod(classPtr, cls_method_parameter_list, curMethod);

							if (methodParameterList)
							{
								jsize parameterLength = java_impl->env->GetArrayLength(methodParameterList);

								for (jsize pIndex = 0; pIndex < parameterLength; pIndex++)
								{
									jobjectArray cparameter = (jobjectArray)java_impl->env->GetObjectArrayElement(methodParameterList, pIndex);

									jstring pName = (jstring)java_impl->env->GetObjectArrayElement(cparameter, 0);
									const char *p_name = java_impl->env->GetStringUTFChars(pName, NULL);

									jstring pSig = (jstring)java_impl->env->GetObjectArrayElement(cparameter, 1);
									const char *p_sig = java_impl->env->GetStringUTFChars(pSig, NULL);

									type pt = java_loader_impl_type(impl, p_name, p_sig);

									if (pt != NULL)
									{
										// Parameter names cannot be inspected with Reflection in Java (https://stackoverflow.com/questions/2237803/can-i-obtain-method-parameter-name-using-java-reflection)
										signature_set(s, (size_t)pIndex, "", pt);
									}
									else
									{
										log_write("metacall", LOG_LEVEL_ERROR, "The parameter type %s in method %s could not be registered, the type is not supported", p_name, m_name);
									}
								}
							}

							// REGISTERING THE METHOD RETURN PARAMETER
							type rt = java_loader_impl_type(impl, m_return_type, m_return_type_sig);

							if (rt != NULL)
							{
								signature_set_return(s, rt);
							}
							else
							{
								log_write("metacall", LOG_LEVEL_ERROR, "The return type %s in method %s could not be registered, the type is not supported", m_return_type, m_name);
							}

							// METHOD REGISTERATION
							if (!strcmp(m_static, "static"))
								class_register_static_method(c, m);
							else
								class_register_method(c, m);
						}
					}

					jmethodID cls_constructor_array = java_impl->env->GetStaticMethodID(classPtr, "java_bootstrap_discover_constructors", "(Ljava/lang/Class;)[Ljava/lang/reflect/Constructor;");

					if (cls_constructor_array != nullptr)
					{
						// TODO: Implement constructors
					}

					scope sp = context_scope(ctx);
					value v = value_create_class(c);

					if (scope_define(sp, cls_name, v) != 0)
					{
						value_type_destroy(v);
						return 1;
					}
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
			log_write("metacall", LOG_LEVEL_ERROR, "JNI failed to attach to the current thread");
		}

		/* Destroy children loaders */
		loader_unload_children(impl);

		java_impl->jvm->DestroyJavaVM();

		delete java_impl;

		return 0;
	}

	return 1;
}