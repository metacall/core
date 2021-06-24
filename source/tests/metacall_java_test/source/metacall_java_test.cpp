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
		const char *java_scripts[] = {
			"Fibonacci.java", "Test.java"
		};

		const enum metacall_value_id fib_int_ids[] = {
			METACALL_INT
		};

		const size_t args_size = sizeof(fib_int_ids) / sizeof(fib_int_ids[0]);

		static const char tag[] = "java";

		ASSERT_EQ((int)0, (int)metacall_load_from_file(tag, java_scripts, sizeof(java_scripts) / sizeof(java_scripts[0]), NULL));

		void *myclass = metacall_class("Test");
		// ASSERT_EQ((enum metacall_value_id)METACALL_CLASS, (enum metacall_value_id)metacall_value_id(myclass_value));
		// void *myclass = metacall_value_to_class(myclass_value);

		// void *myclass_value = metacall_class("Test");
		// ASSERT_EQ((enum metacall_value_id)METACALL_CLASS, (enum metacall_value_id)metacall_value_id(myclass_value));
		// void *myclass = metacall_value_to_class(myclass_value);

		// metacall_value_create_string(john, sizeof(john) - 1), // param1
		// // metacall_value_create_int(10) // param2
		void *constructor_params[] = {
			metacall_value_create_int(10),
			metacall_value_create_int(20)
		};

		void *new_object_v = metacall_class_new(myclass, "Test", constructor_params, sizeof(constructor_params) / sizeof(constructor_params[0]));
		// void *new_object = metacall_value_to_object(new_object_v);

		// void *param2 = metacall_object_get(new_object, "v");
		// ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(param2));
		// ASSERT_EQ((long)10, (long)metacall_value_to_long(param2));

		// metacall_value_destroy(param2);

		// int retcode = metacall_object_set(new_object, "v", metacall_value_create_long(20));
		// ASSERT_EQ((int)0, int(retcode));

		// param2 = metacall_object_get(new_object, "v");
		// ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(param2));
		// ASSERT_EQ((long)20, (long)metacall_value_to_long(param2));

		// metacall_value_destroy(param2);

		// metacall_value_destroy(new_object_v);

		/* TODO: Uncomment this when calls are implemented */
		/*
		void *ret = metacallt_s("fib_impl", fib_int_ids, args_size, 6);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)metacall_value_to_int(ret), (int)8);

		metacall_value_destroy(ret);
		 */

		/* TODO: Test load from memory */
		/*
		static const char buffer[] =
			"...\n"
			"...\n"
			"...\n";

		EXPECT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));
		*/

		/* TODO: Call to the functions of the load from memory script */

		/* TODO: Test load from package */
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
