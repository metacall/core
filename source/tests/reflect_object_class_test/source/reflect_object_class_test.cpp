/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#include <gtest/gtest.h>

#include <reflect/reflect_class.h>
#include <reflect/reflect_object.h>
#include <reflect/reflect_value_type.h>

#include <log/log.h>

typedef struct hello_world_class_type
{
	// These are static attributes that belong to the class
	int a;
	float b;
	char c[10];

} * hello_world_class;

typedef struct hello_world_object_type
{
	// These are attributes that belong to the object
	char d;
	long e;

} * hello_world_object;

int hello_world_object_impl_interface_create(object obj, object_impl impl)
{
	hello_world_object hello_world = (hello_world_object)impl;

	(void)obj;

	EXPECT_NE((void *)NULL, (void *)hello_world);

	// Default values for static attributes (this will be done automatically by the language runtime)
	hello_world->d = 'd';
	hello_world->e = 55;

	return 0;
}

value hello_world_object_impl_interface_get(object obj, object_impl impl, struct accessor_type *accessor)
{
	hello_world_object hello_world = (hello_world_object)impl;
	attribute attr = accessor->data.attr;
	const char *key = attribute_name(attr);

	(void)obj;

	// Horrible but it is just a ilustrative example
	if (strcmp(key, "d") == 0)
	{
		return value_create_char(hello_world->d);
	}
	else if (strcmp(key, "e") == 0)
	{
		return value_create_long(hello_world->e);
	}

	return NULL;
}

int hello_world_object_impl_interface_set(object obj, object_impl impl, struct accessor_type *accessor, value v)
{
	hello_world_object hello_world = (hello_world_object)impl;
	attribute attr = accessor->data.attr;
	const char *key = attribute_name(attr);

	EXPECT_NE((void *)NULL, (void *)hello_world);

	(void)obj;

	// Horrible but it is just a ilustrative example
	if (strcmp(key, "d") == 0)
	{
		hello_world->d = value_to_char(v);
	}
	else if (strcmp(key, "e") == 0)
	{
		hello_world->e = value_to_long(v);
	}

	return 0;
}

value hello_world_object_impl_interface_method_invoke(object obj, object_impl impl, method m, object_args args, size_t size)
{
	// TODO: Maybe we can improve this with other methods and arguments like in reflect_function_test
	static const char str[] = "Hello World";

	(void)obj;
	(void)impl;
	(void)m;
	(void)args;
	(void)size;

	return value_create_string(str, sizeof(str) - 1);
}

value hello_world_object_impl_interface_method_await(object obj, object_impl impl, method m, object_args args, size_t size, object_resolve_callback resolve, object_reject_callback reject, void *ctx)
{
	// TODO
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

int hello_world_object_impl_interface_destructor(object obj, object_impl impl)
{
	(void)obj;
	(void)impl;

	return 0;
}

void hello_world_object_impl_interface_destroy(object obj, object_impl impl)
{
	hello_world_object hello_world_obj = static_cast<hello_world_object>(impl);

	(void)obj;

	delete hello_world_obj;
}

object_interface hello_world_object_impl_interface_singleton()
{
	static struct object_interface_type hello_world_interface
	{
		&hello_world_object_impl_interface_create,
			&hello_world_object_impl_interface_get,
			&hello_world_object_impl_interface_set,
			&hello_world_object_impl_interface_method_invoke,
			&hello_world_object_impl_interface_method_await,
			&hello_world_object_impl_interface_destructor,
			&hello_world_object_impl_interface_destroy
	};

	return &hello_world_interface;
}

int hello_world_class_impl_interface_create(klass cls, class_impl impl)
{
	hello_world_class hello_world = (hello_world_class)impl;

	(void)cls;

	EXPECT_NE((void *)NULL, (void *)hello_world);

	// Default values for static attributes (this will be done automatically by the language runtime)
	hello_world->a = 0;
	hello_world->b = 0.0f;
	hello_world->c[0] = '\0';

	return 0;
}

object hello_world_class_impl_interface_constructor(klass cls, class_impl impl, const char *name, constructor ctor, class_args args, size_t size)
{
	hello_world_object hello_world_obj = new hello_world_object_type();

	(void)impl;
	(void)ctor;

	object obj = object_create(name, ACCESSOR_TYPE_STATIC, hello_world_obj, &hello_world_object_impl_interface_singleton, cls);

	if (object_increment_reference(obj) != 0)
	{
		/* TODO: Abort? */
	}

	if (args == 0)
	{
		// Default constructor
		hello_world_obj->d = 'A';
		hello_world_obj->e = 0L;
	}
	else if (size == 2)
	{
		hello_world_obj->d = value_to_char(args[0]);
		hello_world_obj->e = value_to_long(args[1]);
	}

	return obj;
}

value hello_world_class_impl_interface_static_get(klass cls, class_impl impl, struct accessor_type *accessor)
{
	hello_world_class hello_world = (hello_world_class)impl;
	attribute attr = accessor->data.attr;
	char *key = attribute_name(attr);

	(void)cls;

	if (key == NULL)
	{
		return NULL;
	}

	// Horrible but it is just a ilustrative example
	if (strcmp(key, "a") == 0)
	{
		return value_create_int(hello_world->a);
	}
	else if (strcmp(key, "b") == 0)
	{
		return value_create_float(hello_world->b);
	}
	else if (strcmp(key, "c") == 0)
	{
		return value_create_string(hello_world->c, 9);
	}

	return NULL;
}

int hello_world_class_impl_interface_static_set(klass cls, class_impl impl, struct accessor_type *accessor, value v)
{
	hello_world_class hello_world = (hello_world_class)impl;
	attribute attr = accessor->data.attr;

	EXPECT_NE((void *)NULL, (void *)hello_world);

	(void)cls;

	char *key = attribute_name(attr);

	if (key == NULL)
	{
		return 1;
	}

	// Horrible but it is just a ilustrative example
	if (strcmp(key, "a") == 0)
	{
		hello_world->a = value_to_int(v);
	}
	else if (strcmp(key, "b") == 0)
	{
		hello_world->b = value_to_float(v);
	}
	else if (strcmp(key, "c") == 0)
	{
		strncpy(hello_world->c, value_to_string(v), 9);
	}

	return 0;
}

value hello_world_class_impl_interface_static_invoke(klass cls, class_impl impl, method m, class_args args, size_t argc)
{
	// TODO
	(void)cls;
	(void)impl;
	(void)m;
	(void)args;
	(void)argc;

	return NULL;
}

value hello_world_class_impl_interface_static_await(klass cls, class_impl impl, method m, class_args args, size_t size, class_resolve_callback resolve, class_reject_callback reject, void *ctx)
{
	// TODO
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

void hello_world_class_impl_interface_destroy(klass cls, class_impl impl)
{
	hello_world_class hellow_world_cls = static_cast<hello_world_class>(impl);

	(void)cls;

	delete hellow_world_cls;
}

class_interface hello_world_class_impl_interface_singleton()
{
	static struct class_interface_type hello_world_interface
	{
		&hello_world_class_impl_interface_create,
			&hello_world_class_impl_interface_constructor,
			&hello_world_class_impl_interface_static_get,
			&hello_world_class_impl_interface_static_set,
			&hello_world_class_impl_interface_static_invoke,
			&hello_world_class_impl_interface_static_await,
			&hello_world_class_impl_interface_destroy
	};

	return &hello_world_interface;
}

class reflect_object_class_test : public testing::Test
{
public:
};

TEST_F(reflect_object_class_test, DefaultConstructor)
{
	EXPECT_EQ((int)0, (int)log_configure("metacall",
						  log_policy_format_text(),
						  log_policy_schedule_sync(),
						  log_policy_storage_sequential(),
						  log_policy_stream_stdio(stdout)));

	// Create class
	hello_world_class hellow_world_cls = new hello_world_class_type();

	EXPECT_NE((void *)NULL, (void *)hellow_world_cls);

	klass cls = class_create("HelloWorld", ACCESSOR_TYPE_STATIC, hellow_world_cls, &hello_world_class_impl_interface_singleton);

	EXPECT_EQ((int)class_increment_reference(cls), (int)0);

	// Register constructors
	constructor default_ctor = constructor_create(0, VISIBILITY_PUBLIC);
	constructor custom_ctor = constructor_create(2, VISIBILITY_PUBLIC);

	ASSERT_NE((constructor)NULL, (constructor)default_ctor);
	ASSERT_NE((constructor)NULL, (constructor)custom_ctor);

	type char_type = type_create(TYPE_CHAR, "char", NULL, NULL);
	type long_type = type_create(TYPE_LONG, "long", NULL, NULL);
	type str_type = type_create(TYPE_STRING, "string", NULL, NULL);
	type int_type = type_create(TYPE_INT, "int", NULL, NULL);
	type float_type = type_create(TYPE_FLOAT, "float", NULL, NULL);

	ASSERT_NE((type)NULL, (type)char_type);
	ASSERT_NE((type)NULL, (type)long_type);
	ASSERT_NE((type)NULL, (type)str_type);
	ASSERT_NE((type)NULL, (type)int_type);
	ASSERT_NE((type)NULL, (type)float_type);

	constructor_set(custom_ctor, 0, "c", char_type);
	constructor_set(custom_ctor, 1, "l", long_type);

	EXPECT_EQ((int)0, (int)class_register_constructor(cls, default_ctor));
	EXPECT_EQ((int)0, (int)class_register_constructor(cls, custom_ctor));

	// Register methods
	method test_func_method = method_create(cls, "test_func", 0, NULL, VISIBILITY_PUBLIC, SYNCHRONOUS, NULL);

	ASSERT_NE((method)NULL, (method)test_func_method);

	signature s = method_signature(test_func_method);

	signature_set_return(s, str_type);

	EXPECT_EQ((int)0, (int)class_register_method(cls, test_func_method));

	// Register static attributes
	attribute a_attr = attribute_create(cls, "a", int_type, NULL, VISIBILITY_PUBLIC, NULL);
	attribute b_attr = attribute_create(cls, "b", float_type, NULL, VISIBILITY_PUBLIC, NULL);
	attribute c_attr = attribute_create(cls, "c", str_type, NULL, VISIBILITY_PUBLIC, NULL);

	ASSERT_NE((attribute)NULL, (attribute)a_attr);
	ASSERT_NE((attribute)NULL, (attribute)b_attr);
	ASSERT_NE((attribute)NULL, (attribute)c_attr);

	EXPECT_EQ((int)0, (int)class_register_static_attribute(cls, a_attr));
	EXPECT_EQ((int)0, (int)class_register_static_attribute(cls, b_attr));
	EXPECT_EQ((int)0, (int)class_register_static_attribute(cls, c_attr));

	// Register attributes
	attribute d_attr = attribute_create(cls, "d", char_type, NULL, VISIBILITY_PUBLIC, NULL);
	attribute e_attr = attribute_create(cls, "e", long_type, NULL, VISIBILITY_PUBLIC, NULL);

	ASSERT_NE((attribute)NULL, (attribute)d_attr);
	ASSERT_NE((attribute)NULL, (attribute)e_attr);

	EXPECT_EQ((int)0, (int)class_register_attribute(cls, d_attr));
	EXPECT_EQ((int)0, (int)class_register_attribute(cls, e_attr));

	// Get and set static attributes from the class
	{
		value a = class_static_get(cls, "a");
		ASSERT_NE((value)NULL, (value)a);
		EXPECT_EQ((int)0, (int)value_to_int(a));
		value_type_destroy(a);

		value b = class_static_get(cls, "b");
		ASSERT_NE((value)NULL, (value)b);
		EXPECT_EQ((float)0.0f, (float)value_to_float(b));
		value_type_destroy(b);

		value c = class_static_get(cls, "c");
		ASSERT_NE((value)NULL, (value)c);
		EXPECT_EQ((char)'\0', (char)*value_to_string(c));
		value_type_destroy(c);

		value new_a = value_create_int(1234);
		ASSERT_EQ((int)0, (int)class_static_set(cls, "a", new_a));
		a = class_static_get(cls, "a");
		EXPECT_NE((value)NULL, (value)a);
		EXPECT_EQ((int)1234, (int)value_to_int(a));
		value_type_destroy(a);
		value_type_destroy(new_a);

		value new_c = value_create_string("hi", 2);

		ASSERT_EQ((int)0, (int)class_static_set(cls, "c", new_c));
		c = class_static_get(cls, "c");
		EXPECT_NE((value)NULL, (value)c);
		EXPECT_EQ((char)'h', (char)value_to_string(c)[0]);
		EXPECT_EQ((char)'i', (char)value_to_string(c)[1]);
		value_type_destroy(c);
		value_type_destroy(new_c);
	}

	// Create object (default constructor)
	{
		value args[] = {
			NULL
		};

		type_id ctor_ids[] = {
			TYPE_INVALID
		};

		constructor ctor = class_constructor(cls, ctor_ids, 0);

		ASSERT_EQ((constructor)ctor, (constructor)default_ctor);

		object obj = class_new(cls, "helloWorldObj", ctor, args, 0);
		ASSERT_NE((object)NULL, (object)obj);

		// Get & set attributes from object
		{
			value d = object_get(obj, "d");
			ASSERT_NE((value)NULL, (value)d);
			EXPECT_EQ((char)'d', (char)value_to_char(d));
			value_type_destroy(d);

			value e = object_get(obj, "e");
			ASSERT_NE((value)NULL, (value)e);
			EXPECT_EQ((long)55L, (long)value_to_long(e));
			value_type_destroy(e);

			value new_d = value_create_char('M');
			ASSERT_EQ((char)0, (char)object_set(obj, "d", new_d));
			d = object_get(obj, "d");
			EXPECT_NE((value)NULL, (value)d);
			EXPECT_EQ((char)'M', (char)value_to_char(d));
			value_type_destroy(d);
			value_type_destroy(new_d);

			value new_e = value_create_long(1234);
			ASSERT_EQ((long)0, (long)object_set(obj, "e", new_e));
			e = object_get(obj, "e");
			EXPECT_NE((value)NULL, (value)e);
			EXPECT_EQ((long)1234L, (long)value_to_long(e));
			value_type_destroy(e);
			value_type_destroy(new_e);
		}

		// Test object call
		type_id method_ids[] = {
			TYPE_INVALID
		};

		method m = class_method(cls, "test_func", TYPE_STRING, method_ids, 0);

		ASSERT_EQ((method)m, (method)test_func_method);

		value ret = object_call(obj, m, args, 0);

		ASSERT_NE((value)NULL, (value)ret);

		ASSERT_EQ((int)0, (int)strcmp(value_to_string(ret), "Hello World"));

		value_type_destroy(ret);

		// TODO: Test object await

		object_destroy(obj);
	}

	// Create object (custom constructor)
	{
		value args[] = {
			value_create_char('F'),
			value_create_long(3435L)
		};

		type_id ctor_ids[] = {
			TYPE_CHAR,
			TYPE_LONG
		};

		constructor ctor = class_constructor(cls, ctor_ids, 2);

		ASSERT_EQ((constructor)ctor, (constructor)custom_ctor);

		object obj = class_new(cls, "helloWorldObj", ctor, args, 2);
		ASSERT_NE((object)NULL, (object)obj);

		value_type_destroy(args[0]);
		value_type_destroy(args[1]);

		// Get attributes from object
		{
			value d = object_get(obj, "d");
			ASSERT_NE((value)NULL, (value)d);
			EXPECT_EQ((char)'F', (char)value_to_char(d));
			value_type_destroy(d);

			value e = object_get(obj, "e");
			ASSERT_NE((value)NULL, (value)e);
			EXPECT_EQ((long)3435L, (long)value_to_long(e));
			value_type_destroy(e);
		}

		// Test object call
		type_id method_ids[] = {
			TYPE_INVALID
		};

		method m = class_method(cls, "test_func", TYPE_STRING, method_ids, 0);

		ASSERT_EQ((method)m, (method)test_func_method);

		value ret = object_call(obj, m, args, 0);

		ASSERT_NE((value)NULL, (value)ret);

		ASSERT_EQ((int)0, (int)strcmp(value_to_string(ret), "Hello World"));

		value_type_destroy(ret);

		// TODO: Test object await

		object_destroy(obj);
	}

	type_destroy(char_type);
	type_destroy(long_type);
	type_destroy(str_type);
	type_destroy(int_type);
	type_destroy(float_type);

	class_destroy(cls);
}
