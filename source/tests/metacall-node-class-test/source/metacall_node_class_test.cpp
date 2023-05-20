/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>
#include <metacall/metacall_value.h>

class metacall_node_class_test : public testing::Test
{
public:
};

TEST_F(metacall_node_class_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char *py_scripts[] = {
			"scripts/classname.py"
		};

		EXPECT_EQ((int)0, (int)metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_PY */

/* Node */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		void *new_object_v = NULL;
		{ /* TEST LOAD FROM FILE */
			const char *node_scripts[] = {
				"scripts/node_test.js"
			};

			static const char tag[] = "node";

			ASSERT_EQ((int)0, (int)metacall_load_from_file(tag, node_scripts, sizeof(node_scripts) / sizeof(node_scripts[0]), NULL));
		}

		{ /* TEST STATIC GET, SET AND INVOKE */
			void *myclass = metacall_class("Test");

			void *pyclass_value = metacall("return_class_function");
			ASSERT_EQ((enum metacall_value_id)METACALL_CLASS, (enum metacall_value_id)metacall_value_id(pyclass_value));
			//void *pyclass = metacall_value_to_class(pyclass_value);

			static const char john[] = "John Doe";

			const void *pyconstructor_params[] = {
				metacall_value_create_string(john, sizeof(john) - 1), // param1
				metacall_value_create_int(999999)					  // param2
			};

			void *constructor_params[] = {
				metacall_value_create_int(10),
				metacall_value_create_string("Test String", 11),
				pyclass_value,
				metacall_value_create_array(pyconstructor_params, sizeof(pyconstructor_params) / sizeof(pyconstructor_params[0]))
			};

			new_object_v = metacall_class_new(myclass, "test", constructor_params, sizeof(constructor_params) / sizeof(constructor_params[0]));
			ASSERT_NE((void *)NULL, (void *)new_object_v);

			{ //Invoke static method

				{
					const void *args[] = {
						metacall_value_create_int(1),
						metacall_value_create_int(2),
						metacall_value_create_int(3),
						metacall_value_create_int(4),
						metacall_value_create_int(5),
						metacall_value_create_int(6),
						metacall_value_create_int(7),
						metacall_value_create_int(8),
						metacall_value_create_int(9),
						metacall_value_create_int(10)
					};

					void *argv = metacall_value_create_array(args, sizeof(args) / sizeof(args[0]));

					void *ret = metacallv_class(myclass, "sumArray", &argv, 1);
					ASSERT_EQ((double)55, (double)metacall_value_to_double(ret));

					metacall_value_destroy(ret);
				}

				{
					void *args[] = {
						metacall_value_create_string("Metacall", 8)
					};

					void *ret = metacallv_class(myclass, "hello", args, 1);
					ASSERT_EQ((double)10, (double)metacall_value_to_double(ret));

					metacall_value_destroy(ret);
				}
			}
		}

		{ /* TEST NON STATIC GET, SET and INVOKE FROM CLASS OBJECT */

			void *new_object = metacall_value_to_object(new_object_v);

			{
				void *param1 = metacall_object_get(new_object, "num");
				ASSERT_EQ((double)10, (double)metacall_value_to_double(param1));

				metacall_value_destroy(param1);
			}

			{
				void *param1 = metacall_object_get(new_object, "hello_str");
				ASSERT_EQ((std::string) "hello!", (std::string)metacall_value_to_string(param1));
				metacall_value_destroy(param1);
			}

			{
				void *long_value = metacall_value_create_long(124124L);
				int retcode = metacall_object_set(new_object, "b", long_value);
				metacall_value_destroy(long_value);
				ASSERT_EQ((int)0, int(retcode));

				void *param2 = metacall_object_get(new_object, "b");
				ASSERT_EQ((enum metacall_value_id)METACALL_DOUBLE, (enum metacall_value_id)metacall_value_id(param2));
				ASSERT_EQ((double)124124, (double)metacall_value_to_double(param2));

				metacall_value_destroy(param2);
			}

			//Non Static function Invoke
			{
				void *ret = metacallv_object(new_object, "print", metacall_null_args, 0);
				EXPECT_NE((void *)NULL, (void *)ret);

				metacall_value_destroy(ret);
			}

			{
				void *Fibonacci = metacallv_object(new_object, "return_class", metacall_null_args, 0);
				ASSERT_EQ((enum metacall_value_id)METACALL_CLASS, (enum metacall_value_id)metacall_value_id(Fibonacci));

				metacall_value_destroy(Fibonacci);
			}

			//Value to napi class test
			{
				{
					void *args = metacall_value_create_string("b", 1);
					void *param1 = metacallv_object(new_object, "test_value_to_napi_class_get", &args, 1);
					ASSERT_EQ((double)999999, (double)metacall_value_to_double(param1));
					metacall_value_destroy(param1);
				}

				{
					char str[] = "test value class to napi.";
					void *args[] = {
						metacall_value_create_string("a", 1),
						metacall_value_create_string(str, sizeof(str))
					};

					void *param1 = metacallv_object(new_object, "test_value_to_napi_class_set", args, sizeof(args) / sizeof(args[0]));
					ASSERT_NE((void *)NULL, (void *)param1);
					metacall_value_destroy(param1);

					void *arg = metacall_value_create_string("a", 1);
					void *param2 = metacallv_object(new_object, "test_value_to_napi_class_get", &arg, 1);
					ASSERT_EQ((std::string)str, (std::string)metacall_value_to_string(param2));
					metacall_value_destroy(param2);
				}

				{
					const void *method_args[] = {
						metacall_value_create_int(4),
						metacall_value_create_int(7)
					};

					void *args[] = {
						metacall_value_create_string("check_args", 10),
						metacall_value_create_array(method_args, 2)
					};

					void *param1 = metacallv_object(new_object, "test_value_to_napi_class_method_invoke", args, sizeof(args) / sizeof(args[0]));
					ASSERT_EQ((double)15, (double)metacall_value_to_double(param1));
					metacall_value_destroy(param1);
				}
			}

			metacall_value_destroy(new_object_v);
		}
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

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
