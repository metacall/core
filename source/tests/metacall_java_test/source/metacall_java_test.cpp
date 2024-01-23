/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

			static const char tag[] = "java";

			ASSERT_EQ((int)0, (int)metacall_load_from_file(tag, java_scripts, sizeof(java_scripts) / sizeof(java_scripts[0]), NULL));
		}

		{ /* TEST STATIC GET, SET AND INVOKE */
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
			ASSERT_NE((void *)NULL, (void *)new_object_v);

			metacall_value_destroy(new_object_v);

			{ //GET AND SET
				void *param1 = metacall_class_static_get(myclass, "LONG_TEST");
				ASSERT_EQ((long)20000007, (long)metacall_value_to_long(param1));
				metacall_value_destroy(param1);

				int longSet = metacall_class_static_set(myclass, "LONG_TEST", metacall_value_create_long(2354363575));
				ASSERT_EQ((int)0, int(longSet));

				param1 = metacall_class_static_get(myclass, "LONG_TEST");
				ASSERT_EQ((long)2354363575, (long)metacall_value_to_long(param1));
				metacall_value_destroy(param1);
			}

			{ //GET AND SET
				void *param1 = metacall_class_static_get(myclass, "STRING_TEST");
				ASSERT_EQ((std::string) "Test String", (std::string)metacall_value_to_string(param1));
				metacall_value_destroy(param1);

				int stringSet = metacall_class_static_set(myclass, "STRING_TEST", metacall_value_create_string("ketangupta34", 12));
				ASSERT_EQ((int)0, int(stringSet));

				param1 = metacall_class_static_get(myclass, "STRING_TEST");
				ASSERT_EQ((std::string) "ketangupta34", (std::string)metacall_value_to_string(param1));
				metacall_value_destroy(param1);
			}

			{ //GET CLASS
				void *class_test = metacall_class_static_get(myclass, "CLASS_TEST");
				ASSERT_EQ((void *)metacall_class("Test"), (void *)metacall_value_to_class(class_test));
				metacall_value_destroy(class_test);
			}

			{ //GET ARRAYS
				void *str_test = metacall_class_static_get(myclass, "STRING_TEST_Arr");
				void **str_test_arr = metacall_value_to_array(str_test);
				ASSERT_EQ((int)0, (int)strcmp(metacall_value_to_string(str_test_arr[0]), "Hello"));
				ASSERT_EQ((int)0, (int)strcmp(metacall_value_to_string(str_test_arr[1]), "world"));
				metacall_value_destroy(str_test);

				void *class_test = metacall_class_static_get(myclass, "CLASS_TEST_Arr");
				void **class_test_arr = metacall_value_to_array(class_test);
				ASSERT_EQ((void *)metacall_class("Test"), (void *)metacall_value_to_class(class_test_arr[0]));
				metacall_value_destroy(class_test);
			}

			{ //Invoke
				void *args[] = {
					metacall_value_create_string("Metacall", 8),
					metacall_value_create_int(8)
				};
				void *ret = metacallt_class(myclass, "testFunct", METACALL_INT, args, 2);
				ASSERT_EQ((int)10, (int)metacall_value_to_int(ret));

				metacall_value_destroy(ret);
			}
		}

		{ /* TEST NON STATIC GET, SET and INVOKE FROM CLASS OBJECT */
			void *myclass = metacall_class("Test");

			void *constructor_params[] = {
				metacall_value_create_int(10),
				metacall_value_create_int(20)
			};

			void *new_object_v = metacall_class_new(myclass, "Test", constructor_params, sizeof(constructor_params) / sizeof(constructor_params[0]));
			void *new_object = metacall_value_to_object(new_object_v);

			{
				void *param1 = metacall_object_get(new_object, "INT_NS");
				ASSERT_EQ((int)231, (int)metacall_value_to_int(param1));
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

			{ //Non Static function Invoke
				void *args[] = {
					metacall_value_create_int(8)
				};
				void *ret = metacallt_object(new_object, "TestNonStaticInt", METACALL_INT, args, 1);
				ASSERT_EQ((int)80, (int)metacall_value_to_int(ret));

				metacall_value_destroy(ret);
			}

			metacall_value_destroy(new_object_v);
		}

		{ /* TEST String Array Constructor, GET, SET and Invoke for STATIC  */
			void *myclass = metacall_class("Test");

			const void *constructor_params12[] = {
				metacall_value_create_string("Constructor", 11),
				metacall_value_create_string("Called", 6)
			};
			void *val = metacall_value_create_array(constructor_params12, 2);
			void *constructor_params[] = { val };

			void *new_object_v = metacall_class_new(myclass, "Test", constructor_params, sizeof(constructor_params) / sizeof(constructor_params[0]));
			ASSERT_NE((void *)NULL, (void *)new_object_v);

			metacall_value_destroy(new_object_v);

			{
				void *param1 = metacall_class_static_get(myclass, "INT_TEST_Arr");
				void **array = metacall_value_to_array(param1);
				EXPECT_EQ((int)30, (int)metacall_value_to_int(array[0]));
				EXPECT_EQ((int)12, (int)metacall_value_to_int(array[1]));
				metacall_value_destroy(param1);

				const void *Array_test_array[] = {
					metacall_value_create_int(10),
					metacall_value_create_int(20),
					metacall_value_create_int(30)
				};
				void *val = metacall_value_create_array(Array_test_array, 3);

				int intArrTest = metacall_class_static_set(myclass, "INT_TEST_Arr", val);
				ASSERT_EQ((int)0, int(intArrTest));

				param1 = metacall_class_static_get(myclass, "INT_TEST_Arr");
				array = metacall_value_to_array(param1);
				EXPECT_EQ((int)10, (int)metacall_value_to_int(array[0]));
				EXPECT_EQ((int)20, (int)metacall_value_to_int(array[1]));
				EXPECT_EQ((int)30, (int)metacall_value_to_int(array[2]));
				metacall_value_destroy(param1);
			}

			{
				void *param1 = metacall_class_static_get(myclass, "CHAR_TEST_Arr");
				void **array = metacall_value_to_array(param1);
				EXPECT_EQ((char)'G', (char)metacall_value_to_char(array[1]));
				metacall_value_destroy(param1);

				const void *Array_test_char[] = {
					metacall_value_create_bool('G'),
					metacall_value_create_bool('K')
				};
				void *val = metacall_value_create_array(Array_test_char, 2);

				int boolset = metacall_class_static_set(myclass, "CHAR_TEST_Arr", val);
				ASSERT_EQ((int)0, int(boolset));

				param1 = metacall_class_static_get(myclass, "CHAR_TEST_Arr");
				array = metacall_value_to_array(param1);
				EXPECT_EQ((char)'K', (char)metacall_value_to_char(array[1]));

				metacall_value_destroy(param1);
			}

			{
				void *param1 = metacall_class_static_get(myclass, "STRING_TEST_Arr");
				void **array = metacall_value_to_array(param1);
				EXPECT_EQ((std::string) "Hello", (std::string)metacall_value_to_string(array[0]));
				EXPECT_EQ((std::string) "world", (std::string)metacall_value_to_string(array[1]));
				metacall_value_destroy(param1);

				const void *Array_test_string[] = {
					metacall_value_create_string("abc", 3),
					metacall_value_create_string("def", 3),
					metacall_value_create_string("ghi", 3)
				};
				void *val = metacall_value_create_array(Array_test_string, 3);

				int boolset = metacall_class_static_set(myclass, "STRING_TEST_Arr", val);
				ASSERT_EQ((int)0, int(boolset));

				param1 = metacall_class_static_get(myclass, "STRING_TEST_Arr");
				array = metacall_value_to_array(param1);
				EXPECT_EQ((std::string) "abc", (std::string)metacall_value_to_string(array[0]));
				EXPECT_EQ((std::string) "def", (std::string)metacall_value_to_string(array[1]));
				EXPECT_EQ((std::string) "ghi", (std::string)metacall_value_to_string(array[2]));

				metacall_value_destroy(param1);
			}

			{ // Testing MAIN function call
				void *args[] = {
					metacall_value_create_array({}, 0)
				};

				void *ret = metacallv_class(myclass, "main", args, 1);
				metacall_value_destroy(ret);
			}

			{
				const void *Array_test_int[] = {
					metacall_value_create_int(2),
					metacall_value_create_int(3),
					metacall_value_create_int(5)
				};

				void *args[] = {
					metacall_value_create_array(Array_test_int, 3)
				};

				void *ret = metacallt_class(myclass, "testIntArrayAdd", METACALL_INT, args, 1);
				EXPECT_EQ((int)10, (int)metacall_value_to_int(ret));
			}

			// {
			// 	const void *Array_test_string[] = {
			// 		metacall_value_create_string("aaaa", 4),
			// 		metacall_value_create_string("bbbb", 4),
			// 		metacall_value_create_string("cccc", 4)
			// 	};
			// 	void *args[] = {
			// 		metacall_value_create_array(Array_test_string, 3)
			// 	};

			// 	void *ret = metacallt_class(myclass, "testStringArrayFunction", METACALL_ARRAY, args, 1);
			// 	void **array = metacall_value_to_array(ret);
			// 	EXPECT_EQ((std::string) "test", (std::string)metacall_value_to_string(array[0]));
			// 	EXPECT_EQ((std::string) "worked", (std::string)metacall_value_to_string(array[1]));
			// }
		}

		{ /* TEST Array GET, SET and Invoke for NON STATIC  */
			void *myclass = metacall_class("Test");

			const void *constructor_params12[] = {
				metacall_value_create_string("Constructor", 11),
				metacall_value_create_string("Called", 6)
			};
			void *val = metacall_value_create_array(constructor_params12, 2);
			void *constructor_params[] = { val };

			void *new_object_v = metacall_class_new(myclass, "Test", constructor_params, sizeof(constructor_params) / sizeof(constructor_params[0]));
			void *new_object = metacall_value_to_object(new_object_v);

			{
				void *param1 = metacall_object_get(new_object, "INT_TEST_Arr_NS");
				void **array = metacall_value_to_array(param1);
				EXPECT_EQ((int)50, (int)metacall_value_to_int(array[0]));
				EXPECT_EQ((int)100, (int)metacall_value_to_int(array[1]));
				metacall_value_destroy(param1);

				const void *Array_test_array[] = {
					metacall_value_create_int(10),
					metacall_value_create_int(20),
					metacall_value_create_int(30)
				};
				void *val = metacall_value_create_array(Array_test_array, 3);

				int intArrTest = metacall_object_set(new_object, "INT_TEST_Arr_NS", val);
				ASSERT_EQ((int)0, int(intArrTest));

				param1 = metacall_object_get(new_object, "INT_TEST_Arr_NS");
				array = metacall_value_to_array(param1);
				EXPECT_EQ((int)10, (int)metacall_value_to_int(array[0]));
				EXPECT_EQ((int)20, (int)metacall_value_to_int(array[1]));
				EXPECT_EQ((int)30, (int)metacall_value_to_int(array[2]));
				metacall_value_destroy(param1);
			}

			metacall_value_destroy(new_object_v);
		}

		{ /* TEST LOAD FROM PACKAGE get, set and invoke for Static and Non Static */
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

			metacall_value_destroy(new_object_v);
		}

		// { // Test load from memory
		// 	static const char buffer[] =
		// 		"public class memoryTest{"
		// 		"public static String memoryString = \"Memory test string\";"
		// 		"}";

		// 	EXPECT_EQ((int)0, (int)metacall_load_from_memory("java", buffer, sizeof(buffer), NULL));
		// }
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

		std::cout << inspect_str << std::endl;

		metacall_allocator_free(allocator, inspect_str);

		metacall_allocator_destroy(allocator);
	}

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
