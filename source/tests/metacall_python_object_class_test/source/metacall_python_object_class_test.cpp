/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

#include <gtest/gtest.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

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
				metacall_value_create_string(john, sizeof(john) - 1), // param1
				metacall_value_create_int(999999)					  // param2
			};
			void *new_object_v = metacall_class_new(myclass, "objectname", constructor_params, sizeof(constructor_params) / sizeof(constructor_params[0]));
			metacall_value_destroy(constructor_params[0]);
			metacall_value_destroy(constructor_params[1]);
			void *new_object = metacall_value_to_object(new_object_v);

			void *param2 = metacall_object_get(new_object, "b");
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(param2));
			ASSERT_EQ((long)999999, (long)metacall_value_to_long(param2));

			metacall_value_destroy(param2);

			int retcode = metacall_object_set(new_object, "b", metacall_value_create_long(124124L));
			ASSERT_EQ((int)0, int(retcode));

			param2 = metacall_object_get(new_object, "b");
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(param2));
			ASSERT_EQ((long)124124L, (long)metacall_value_to_long(param2));

			metacall_value_destroy(param2);

			metacall_value_destroy(new_object_v);
		}

		{
			void *myclass_value = metacall("return_class_function");
			ASSERT_EQ((enum metacall_value_id)METACALL_CLASS, (enum metacall_value_id)metacall_value_id(myclass_value));
			void *myclass = metacall_value_to_class(myclass_value);

			void *param2 = metacall_class_static_get(myclass, "b");
			ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(param2));
			ASSERT_EQ((long)20L, (long)metacall_value_to_long(param2));

			metacall_value_destroy(param2);

			int retcode = metacall_class_static_set(myclass, "b", metacall_value_create_long(44444L));
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

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
