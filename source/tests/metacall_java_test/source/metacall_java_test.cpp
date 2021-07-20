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
#include <metacall/metacall_value.h>

class metacall_java_test : public testing::Test
{
public:
};

TEST_F(metacall_java_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Java */
#if defined(OPTION_BUILD_LOADERS_JAVA)
	{
		{ /* TEST LOAD FROM FILE */
			const char *java_scripts[] = {
				"Fibonacci.java", "Test.java"
			};

			const enum metacall_value_id fib_int_ids[] = {
				METACALL_INT
			};

			const size_t args_size = sizeof(fib_int_ids) / sizeof(fib_int_ids[0]);

			static const char tag[] = "java";

			ASSERT_EQ((int)0, (int)metacall_load_from_file(tag, java_scripts, sizeof(java_scripts) / sizeof(java_scripts[0]), NULL));
		}

		{ /* TEST STATIC GET AND SET */
			void *myclass = metacall_class("Test");

			void *constructor_params[] = {
				metacall_value_create_bool(false),
				metacall_value_create_char('H'),
				metacall_value_create_short(200),
				metacall_value_create_int(10),
				metacall_value_create_long(20000007),
				metacall_value_create_float(20.321),
				metacall_value_create_double(200.123456789),
				metacall_value_create_string("Test String", 11)
			};

			void *new_object_v = metacall_class_new(myclass, "Test", constructor_params, sizeof(constructor_params) / sizeof(constructor_params[0]));

			{
				void *param1 = metacall_class_static_get(myclass, "BOOL_TEST");
				ASSERT_EQ((bool)false, (bool)metacall_value_to_bool(param1));
				metacall_value_destroy(param1);

				int boolset = metacall_class_static_set(myclass, "BOOL_TEST", metacall_value_create_bool(true));
				ASSERT_EQ((int)0, int(boolset));

				param1 = metacall_class_static_get(myclass, "BOOL_TEST");
				ASSERT_EQ((bool)true, (bool)metacall_value_to_bool(param1));
				metacall_value_destroy(param1);
			}

			{
				void *param1 = metacall_class_static_get(myclass, "LONG_TEST");
				ASSERT_EQ((long)20000007, (long)metacall_value_to_long(param1));
				metacall_value_destroy(param1);

				int longSet = metacall_class_static_set(myclass, "LONG_TEST", metacall_value_create_long(2354363575));
				ASSERT_EQ((int)0, int(longSet));

				param1 = metacall_class_static_get(myclass, "LONG_TEST");
				ASSERT_EQ((long)2354363575, (long)metacall_value_to_long(param1));
				metacall_value_destroy(param1);
			}

			{
				void *param1 = metacall_class_static_get(myclass, "STRING_TEST");
				ASSERT_EQ((std::string) "Test String", (std::string)metacall_value_to_string(param1));
				metacall_value_destroy(param1);

				int stringSet = metacall_class_static_set(myclass, "STRING_TEST", metacall_value_create_string("ketangupta34", 12));
				ASSERT_EQ((int)0, int(stringSet));

				param1 = metacall_class_static_get(myclass, "STRING_TEST");
				ASSERT_EQ((std::string) "ketangupta34", (std::string)metacall_value_to_string(param1));
				metacall_value_destroy(param1);
			}
		}

		{ /* TEST NON STATIC GET AND SET FROM CLASS OBJECT */
			void *myclass = metacall_class("Test");

			void *constructor_params[] = {
				metacall_value_create_int(10),
				metacall_value_create_int(20)
			};

			void *new_object_v = metacall_class_new(myclass, "Test", constructor_params, sizeof(constructor_params) / sizeof(constructor_params[0]));

			void *new_object = metacall_value_to_object(new_object_v);

			{
				void *param1 = metacall_object_get(new_object, "INT_NS");
				ASSERT_EQ((int)123433, (int)metacall_value_to_int(param1));
				metacall_value_destroy(param1);

				int intobjset = metacall_object_set(new_object, "INT_NS", metacall_value_create_int(4321));
				ASSERT_EQ((int)0, int(intobjset));

				param1 = metacall_object_get(new_object, "INT_NS");
				ASSERT_EQ((int)4321, (int)metacall_value_to_int(param1));
				metacall_value_destroy(param1);
			}

			{
				void *param1 = metacall_object_get(new_object, "CHAR_NS");
				ASSERT_EQ((char)'Z', (char)metacall_value_to_char(param1));
				metacall_value_destroy(param1);

				int charobjset = metacall_object_set(new_object, "CHAR_NS", metacall_value_create_char('Y'));
				ASSERT_EQ((int)0, int(charobjset));

				param1 = metacall_object_get(new_object, "CHAR_NS");
				ASSERT_EQ((char)'Y', (char)metacall_value_to_char(param1));
				metacall_value_destroy(param1);
			}

			{
				void *param1 = metacall_object_get(new_object, "STRING_NS");
				ASSERT_EQ((std::string) "NS string", (std::string)metacall_value_to_string(param1));
				metacall_value_destroy(param1);

				int strobjset = metacall_object_set(new_object, "STRING_NS", metacall_value_create_string("UPDATED", 7));
				ASSERT_EQ((int)0, int(strobjset));

				param1 = metacall_object_get(new_object, "STRING_NS");
				ASSERT_EQ((std::string) "UPDATED", (std::string)metacall_value_to_string(param1));
				metacall_value_destroy(param1);
			}

			metacall_value_destroy(new_object_v);
		}

		{ /* TEST LOAD FROM PACKAGE */
			ASSERT_EQ((int)0, (int)metacall_load_from_package("java", "JarTest.jar", NULL));

			void *myclass = metacall_class("src.JarTest.TestJar");

			void *new_object_v = metacall_class_new(myclass, "src.JarTest.TestJar", {}, 0);

			{
				void *param = metacall_class_static_get(myclass, "jarTestString");
				ASSERT_EQ((std::string) "This is a static Jar String", (std::string)metacall_value_to_string(param));
				metacall_value_destroy(param);

				int stringSet = metacall_class_static_set(myclass, "jarTestString", metacall_value_create_string("ketangupta34", 12));
				ASSERT_EQ((int)0, int(stringSet));

				param = metacall_class_static_get(myclass, "jarTestString");
				ASSERT_EQ((std::string) "ketangupta34", (std::string)metacall_value_to_string(param));
				metacall_value_destroy(param);
			}
		}

		/* TODO: Test load from memory */
		/*
		static const char buffer[] =
			"...\n"
			"...\n"
			"...\n";

		EXPECT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));
		*/

		/* TODO: Call to the functions of the load from memory script */

		/* TODO: Uncomment this when calls are implemented */
		/*
		void *ret = metacallt_s("fib_impl", fib_int_ids, args_size, 6);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)metacall_value_to_int(ret), (int)8);

		metacall_value_destroy(ret);
		 */
	}
#endif /* OPTION_BUILD_LOADERS_JAVA */

	/* Print inspect information */
	{
		size_t size = 0;

		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		char *inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char *)NULL, (char *)inspect_str);

		EXPECT_GT((size_t)size, (size_t)0);

		metacall_allocator_free(allocator, inspect_str);

		metacall_allocator_destroy(allocator);
	}

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
