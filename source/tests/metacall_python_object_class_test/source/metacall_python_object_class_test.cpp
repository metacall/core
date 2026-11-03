/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

#include <reflect/reflect_attribute.h>
#include <reflect/reflect_class.h>
#include <reflect/reflect_method.h>
#include <reflect/reflect_object.h>
#include <reflect/reflect_value_type.h>

#include <cstring>

/* Class implemented in C, used for testing the conversion of classes that do not come from Python */
struct foreign_class_type
{
	long x;
};

struct foreign_object_type
{
	long y;
};

static int foreign_object_interface_create(object obj, object_impl impl)
{
	(void)obj;
	(void)impl;

	return 0;
}

static value foreign_object_interface_get(object obj, object_impl impl, struct accessor_type *accessor)
{
	(void)obj;

	if (strcmp(attribute_name(accessor->data.attr), "y") == 0)
	{
		return value_create_long(static_cast<foreign_object_type *>(impl)->y);
	}

	return NULL;
}

static int foreign_object_interface_set(object obj, object_impl impl, struct accessor_type *accessor, value v)
{
	(void)obj;

	if (strcmp(attribute_name(accessor->data.attr), "y") == 0 && value_type_id(v) == TYPE_LONG)
	{
		static_cast<foreign_object_type *>(impl)->y = value_to_long(v);
		return 0;
	}

	return 1;
}

static value foreign_object_interface_method_invoke(object obj, object_impl impl, method m, object_args args, size_t argc)
{
	(void)obj;

	if (strcmp(method_name(m), "add") == 0 && argc == 1 && value_type_id(args[0]) == TYPE_LONG)
	{
		return value_create_long(static_cast<foreign_object_type *>(impl)->y + value_to_long(args[0]));
	}

	return NULL;
}

static value foreign_object_interface_method_await(object obj, object_impl impl, method m, object_args args, size_t argc, object_resolve_callback resolve, object_reject_callback reject, void *ctx)
{
	(void)obj;
	(void)impl;
	(void)m;
	(void)args;
	(void)argc;
	(void)resolve;
	(void)reject;
	(void)ctx;

	return NULL;
}

static int foreign_object_interface_destructor(object obj, object_impl impl)
{
	(void)obj;
	(void)impl;

	return 0;
}

static void foreign_object_interface_destroy(object obj, object_impl impl)
{
	(void)obj;

	delete static_cast<foreign_object_type *>(impl);
}

static object_interface foreign_object_interface_singleton(void)
{
	static struct object_interface_type foreign_object_interface = {
		&foreign_object_interface_create,
		&foreign_object_interface_get,
		&foreign_object_interface_set,
		&foreign_object_interface_method_invoke,
		&foreign_object_interface_method_await,
		&foreign_object_interface_destructor,
		&foreign_object_interface_destroy
	};

	return &foreign_object_interface;
}

static int foreign_class_interface_create(klass cls, class_impl impl)
{
	(void)cls;
	(void)impl;

	return 0;
}

static object foreign_class_interface_constructor(klass cls, class_impl impl, const char *name, constructor ctor, class_args args, size_t argc)
{
	(void)impl;
	(void)ctor;

	foreign_object_type *foreign_obj = new foreign_object_type();

	foreign_obj->y = (argc == 1 && value_type_id(args[0]) == TYPE_LONG) ? value_to_long(args[0]) : 0L;

	object obj = object_create(name, ACCESSOR_TYPE_STATIC, foreign_obj, &foreign_object_interface_singleton, cls);

	if (obj == NULL)
	{
		delete foreign_obj;
	}

	return obj;
}

static value foreign_class_interface_static_get(klass cls, class_impl impl, struct accessor_type *accessor)
{
	(void)cls;

	if (strcmp(attribute_name(accessor->data.attr), "x") == 0)
	{
		return value_create_long(static_cast<foreign_class_type *>(impl)->x);
	}

	return NULL;
}

static int foreign_class_interface_static_set(klass cls, class_impl impl, struct accessor_type *accessor, value v)
{
	(void)cls;

	if (strcmp(attribute_name(accessor->data.attr), "x") == 0 && value_type_id(v) == TYPE_LONG)
	{
		static_cast<foreign_class_type *>(impl)->x = value_to_long(v);
		return 0;
	}

	return 1;
}

static value foreign_class_interface_static_invoke(klass cls, class_impl impl, method m, class_args args, size_t argc)
{
	(void)cls;
	(void)impl;

	if (strcmp(method_name(m), "twice") == 0 && argc == 1 && value_type_id(args[0]) == TYPE_LONG)
	{
		return value_create_long(value_to_long(args[0]) * 2L);
	}

	return NULL;
}

static value foreign_class_interface_static_await(klass cls, class_impl impl, method m, class_args args, size_t argc, class_resolve_callback resolve, class_reject_callback reject, void *ctx)
{
	(void)cls;
	(void)impl;
	(void)m;
	(void)args;
	(void)argc;
	(void)resolve;
	(void)reject;
	(void)ctx;

	return NULL;
}

static void foreign_class_interface_destroy(klass cls, class_impl impl)
{
	(void)cls;

	delete static_cast<foreign_class_type *>(impl);
}

static class_interface foreign_class_interface_singleton(void)
{
	static struct class_interface_type foreign_class_interface = {
		&foreign_class_interface_create,
		&foreign_class_interface_constructor,
		&foreign_class_interface_static_get,
		&foreign_class_interface_static_set,
		&foreign_class_interface_static_invoke,
		&foreign_class_interface_static_await,
		&foreign_class_interface_destroy
	};

	return &foreign_class_interface;
}

class metacall_python_class_test : public testing::Test
{
public:
};

TEST_F(metacall_python_class_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char *py_scripts[] = {
			"classname.py"
		};

		EXPECT_EQ((int)0, (int)metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		{
			void *myclass_value = metacall("return_class_function");
			ASSERT_EQ((enum metacall_value_id)METACALL_CLASS, (enum metacall_value_id)metacall_value_id(myclass_value));
			void *return_itself_args[] = {
				myclass_value
			};
			void *return_itself = metacallv("return_itself", return_itself_args);
			ASSERT_EQ((enum metacall_value_id)METACALL_CLASS, (enum metacall_value_id)metacall_value_id(return_itself));
			metacall_value_destroy(return_itself);
			metacall_value_destroy(myclass_value);
		}

		/* Class that does not come from Python (it must be wrapped into a Python type) */
		{
			static const char foreign_script[] =
				"def foreign_identity(cls):\n"
				"	return cls\n"
				"def foreign_same(a, b):\n"
				"	return a is b\n"
				"def foreign_is_class(cls):\n"
				"	import inspect\n"
				"	return inspect.isclass(cls) and cls.__name__ == 'ForeignClass'\n"
				"def foreign_static_get(cls):\n"
				"	return cls.x\n"
				"def foreign_static_set(cls, v):\n"
				"	cls.x = v\n"
				"	return cls.x\n"
				"def foreign_static_call(cls, v):\n"
				"	return cls.twice(v) + cls.twice(v)\n"
				"def foreign_construct(cls):\n"
				"	return cls(5).y + cls().y\n"
				"def foreign_instance(cls):\n"
				"	o = cls(5)\n"
				"	o.y = 7\n"
				"	m = o.add\n"
				"	return o.add(3) + m(1) + o.y\n"
				"def foreign_isinstance(cls):\n"
				"	o = cls(1)\n"
				"	return isinstance(o, cls) and type(o) is cls\n"
				"def foreign_new_object(cls):\n"
				"	return cls(42)\n"
				"def foreign_object_get(o):\n"
				"	return o.y\n"
				"def foreign_raises(f):\n"
				"	try:\n"
				"		f()\n"
				"	except TypeError:\n"
				"		return True\n"
				"	return False\n"
				"def foreign_errors(cls):\n"
				"	o = cls(1)\n"
				"	return foreign_raises(lambda: o.add(1, x=2)) and foreign_raises(lambda: cls.twice(1, x=2)) and foreign_raises(lambda: cls(y=1))\n";

			EXPECT_EQ((int)0, (int)metacall_load_from_memory("py", foreign_script, sizeof(foreign_script), NULL));

			foreign_class_type *foreign_impl = new foreign_class_type();
			foreign_impl->x = 10L;

			klass foreign_cls = class_create("ForeignClass", ACCESSOR_TYPE_STATIC, foreign_impl, &foreign_class_interface_singleton);
			ASSERT_NE((klass)NULL, (klass)foreign_cls);

			attribute x_attr = attribute_create(foreign_cls, "x", NULL, NULL, VISIBILITY_PUBLIC, NULL);
			ASSERT_EQ((int)0, (int)class_register_static_attribute(foreign_cls, x_attr));

			method twice_method = method_create(foreign_cls, "twice", 1, NULL, VISIBILITY_PUBLIC, SYNCHRONOUS, NULL);
			ASSERT_EQ((int)0, (int)class_register_static_method(foreign_cls, twice_method));

			constructor ctor = constructor_create(1, VISIBILITY_PUBLIC);
			constructor_set(ctor, 0, "y", NULL);
			ASSERT_EQ((int)0, (int)class_register_constructor(foreign_cls, ctor));

			attribute y_attr = attribute_create(foreign_cls, "y", NULL, NULL, VISIBILITY_PUBLIC, NULL);
			ASSERT_EQ((int)0, (int)class_register_attribute(foreign_cls, y_attr));

			method add_method = method_create(foreign_cls, "add", 1, NULL, VISIBILITY_PUBLIC, SYNCHRONOUS, NULL);
			ASSERT_EQ((int)0, (int)class_register_method(foreign_cls, add_method));

			void *foreign_value = metacall_value_create_class(foreign_cls);

			void *identity_args[] = { foreign_value };
			void *ret = metacallv("foreign_identity", identity_args);
			ASSERT_EQ((enum metacall_value_id)METACALL_CLASS, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((void *)foreign_cls, (void *)metacall_value_to_class(ret));
			metacall_value_destroy(ret);

			void *same_args[] = { foreign_value, foreign_value };
			ret = metacallv("foreign_same", same_args);
			ASSERT_EQ((enum metacall_value_id)METACALL_BOOL, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((boolean)1L, (boolean)metacall_value_to_bool(ret));
			metacall_value_destroy(ret);

			ret = metacallv("foreign_is_class", identity_args);
			ASSERT_EQ((enum metacall_value_id)METACALL_BOOL, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((boolean)1L, (boolean)metacall_value_to_bool(ret));
			metacall_value_destroy(ret);

			ret = metacallv("foreign_static_get", identity_args);
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((long)10L, (long)metacall_value_to_long(ret));
			metacall_value_destroy(ret);

			void *set_args[] = { foreign_value, metacall_value_create_long(33L) };
			ret = metacallv("foreign_static_set", set_args);
			metacall_value_destroy(set_args[1]);
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((long)33L, (long)metacall_value_to_long(ret));
			EXPECT_EQ((long)33L, (long)foreign_impl->x);
			metacall_value_destroy(ret);

			void *call_args[] = { foreign_value, metacall_value_create_long(4L) };
			ret = metacallv("foreign_static_call", call_args);
			metacall_value_destroy(call_args[1]);
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((long)16L, (long)metacall_value_to_long(ret));
			metacall_value_destroy(ret);

			ret = metacallv("foreign_construct", identity_args);
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((long)5L, (long)metacall_value_to_long(ret));
			metacall_value_destroy(ret);

			ret = metacallv("foreign_instance", identity_args);
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((long)25L, (long)metacall_value_to_long(ret));
			metacall_value_destroy(ret);

			ret = metacallv("foreign_errors", identity_args);
			ASSERT_EQ((enum metacall_value_id)METACALL_BOOL, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((boolean)1L, (boolean)metacall_value_to_bool(ret));
			metacall_value_destroy(ret);

			ret = metacallv("foreign_isinstance", identity_args);
			ASSERT_EQ((enum metacall_value_id)METACALL_BOOL, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((boolean)1L, (boolean)metacall_value_to_bool(ret));
			metacall_value_destroy(ret);

			/* Object created in Python from a foreign class, it must be the foreign object when it comes back */
			void *foreign_obj_value = metacallv("foreign_new_object", identity_args);
			ASSERT_EQ((enum metacall_value_id)METACALL_OBJECT, (enum metacall_value_id)metacall_value_id(foreign_obj_value));
			void *foreign_obj = metacall_value_to_object(foreign_obj_value);
			EXPECT_EQ((void *)foreign_cls, (void *)object_class_get((object)foreign_obj));

			ret = metacall_object_get(foreign_obj, "y");
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((long)42L, (long)metacall_value_to_long(ret));
			metacall_value_destroy(ret);

			void *foreign_obj_args[] = { foreign_obj_value };
			ret = metacallv("foreign_identity", foreign_obj_args);
			ASSERT_EQ((enum metacall_value_id)METACALL_OBJECT, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((void *)foreign_obj, (void *)metacall_value_to_object(ret));
			metacall_value_destroy(ret);

			void *foreign_obj_same_args[] = { foreign_obj_value, foreign_obj_value };
			ret = metacallv("foreign_same", foreign_obj_same_args);
			ASSERT_EQ((enum metacall_value_id)METACALL_BOOL, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((boolean)1L, (boolean)metacall_value_to_bool(ret));
			metacall_value_destroy(ret);

			ret = metacallv("foreign_object_get", foreign_obj_args);
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((long)42L, (long)metacall_value_to_long(ret));
			metacall_value_destroy(ret);

			metacall_value_destroy(foreign_obj_value);

			metacall_value_destroy(foreign_value);
		}

		{
			void *myobject_value = metacall("return_object_function");
			ASSERT_EQ((enum metacall_value_id)METACALL_OBJECT, (enum metacall_value_id)metacall_value_id(myobject_value));
			void *return_itself_args[] = {
				myobject_value
			};
			void *return_itself = metacallv("return_itself", return_itself_args);
			ASSERT_EQ((enum metacall_value_id)METACALL_OBJECT, (enum metacall_value_id)metacall_value_id(return_itself));
			metacall_value_destroy(return_itself);
			metacall_value_destroy(myobject_value);
		}

		{
			void *ret = metacall("function_returns_object_new_local_variable");
			ASSERT_EQ((enum metacall_value_id)METACALL_OBJECT, (enum metacall_value_id)metacall_value_id(ret));
			metacall_value_destroy(ret);
		}

		{
			void *myclass_value = metacall("return_class_function");
			ASSERT_EQ((enum metacall_value_id)METACALL_CLASS, (enum metacall_value_id)metacall_value_id(myclass_value));
			void *myclass = metacall_value_to_class(myclass_value);

			static const char john[] = "John Doe";

			void *constructor_params[] = {
				metacall_value_create_string(john, sizeof(john) - 1),
				metacall_value_create_int(999999)
			};
			void *new_object_v = metacall_class_new(myclass, "objectname", constructor_params, sizeof(constructor_params) / sizeof(constructor_params[0]));
			metacall_value_destroy(constructor_params[0]);
			metacall_value_destroy(constructor_params[1]);
			void *new_object = metacall_value_to_object(new_object_v);

			void *param2 = metacall_object_get(new_object, "b");
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(param2));
			ASSERT_EQ((long)999999, (long)metacall_value_to_long(param2));

			metacall_value_destroy(param2);

			void *long_value = metacall_value_create_long(124124L);
			int retcode = metacall_object_set(new_object, "b", long_value);
			metacall_value_destroy(long_value);
			ASSERT_EQ((int)0, int(retcode));

			param2 = metacall_object_get(new_object, "b");
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(param2));
			ASSERT_EQ((long)124124L, (long)metacall_value_to_long(param2));

			metacall_value_destroy(param2);

			metacall_value_destroy(new_object_v);
			metacall_value_destroy(myclass_value);
		}

		{
			void *myclass_value = metacall("return_class_function");
			ASSERT_EQ((enum metacall_value_id)METACALL_CLASS, (enum metacall_value_id)metacall_value_id(myclass_value));
			void *myclass = metacall_value_to_class(myclass_value);

			void *param2 = metacall_class_static_get(myclass, "b");
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(param2));
			ASSERT_EQ((long)20L, (long)metacall_value_to_long(param2));

			metacall_value_destroy(param2);

			void *long_value = metacall_value_create_long(44444L);
			int retcode = metacall_class_static_set(myclass, "b", long_value);
			metacall_value_destroy(long_value);
			ASSERT_EQ((int)0, int(retcode));

			param2 = metacall_class_static_get(myclass, "b");
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(param2));
			ASSERT_EQ((long)44444L, (long)metacall_value_to_long(param2));

			metacall_value_destroy(param2);

			metacall_value_destroy(myclass_value);
		}

		{
			void *myclass = metacall_class("MyClass");
			ASSERT_NE((void *)NULL, (void *)myclass);

			static const char works[] = "It works!";

			void *static_method_args[] = {
				metacall_value_create_string(works, sizeof(works) - 1)
			};
			void *ret_value = metacallv_class(myclass, "static", static_method_args, sizeof(static_method_args) / sizeof(static_method_args[0]));

			ASSERT_EQ((enum metacall_value_id)METACALL_STRING, (enum metacall_value_id)metacall_value_id(ret_value));
			metacall_value_destroy(static_method_args[0]);
			metacall_value_destroy(ret_value);

			// Get and Set
			void *param2 = metacall_class_static_get(myclass, "b");
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(param2));
			ASSERT_EQ((long)44444L, (long)metacall_value_to_long(param2));

			metacall_value_destroy(param2);

			void *long_value = metacall_value_create_long(5555L);
			int retcode = metacall_class_static_set(myclass, "b", long_value);
			metacall_value_destroy(long_value);
			ASSERT_EQ((int)0, int(retcode));

			param2 = metacall_class_static_get(myclass, "b");
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(param2));
			ASSERT_EQ((long)5555L, (long)metacall_value_to_long(param2));

			metacall_value_destroy(param2);
		}

		{
			void *obj_value = metacall("return_object_function");
			ASSERT_EQ((enum metacall_value_id)METACALL_OBJECT, (enum metacall_value_id)metacall_value_id(obj_value));
			void *obj = metacall_value_to_object(obj_value);

			static const char john[] = "John Doe";

			void *return_bye_args[] = {
				metacall_value_create_string(john, sizeof(john) - 1)
			};
			void *ret = metacallv_object(obj, "return_bye", return_bye_args, sizeof(return_bye_args) / sizeof(return_bye_args[0]));
			metacall_value_destroy(return_bye_args[0]);
			ASSERT_EQ((enum metacall_value_id)METACALL_STRING, (enum metacall_value_id)metacall_value_id(ret));
			metacall_value_destroy(ret);

			void *return_check_args[] = {
				metacall_value_create_long(4L),
				metacall_value_create_long(7L)
			};
			ret = metacallv_object(obj, "check_args", return_check_args, sizeof(return_check_args) / sizeof(return_check_args[0]));
			metacall_value_destroy(return_check_args[0]);
			metacall_value_destroy(return_check_args[1]);
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(ret));
			ASSERT_EQ((long)15L, (long)metacall_value_to_long(ret));
			metacall_value_destroy(ret);

			metacall_value_destroy(obj_value);
		}
	}
#endif /* OPTION_BUILD_LOADERS_PY */

	/* Print inspect information */
	{
		size_t size = 0;

		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		char *inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char *)NULL, (char *)inspect_str);

		EXPECT_GT((size_t)size, (size_t)0);

		std::cout << inspect_str << std::endl;

		metacall_allocator_free(allocator, inspect_str);

		metacall_allocator_destroy(allocator);
	}

	metacall_destroy();
}
