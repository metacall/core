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

class metacall_handle_get_test : public testing::Test
{
public:
};

TEST_F(metacall_handle_get_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const char *node_scripts[] = {
			"nod.js"
		};

		void *handle = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("node", node_scripts, sizeof(node_scripts) / sizeof(node_scripts[0]), &handle));

		ASSERT_NE((void *)NULL, (void *)handle);

		void *func = metacall_handle_function(handle, "call_test");

		ASSERT_NE((void *)NULL, (void *)func);

		const enum metacall_value_id double_ids[] = {
			METACALL_DOUBLE, METACALL_DOUBLE
		};

		void *ret = metacallht_s(handle, "call_test", double_ids, 2, 10.0, 2.0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)20.0);

		metacall_value_destroy(ret);

		void *args[] = {
			metacall_value_create_double(255.0),
			metacall_value_create_double(5.0)
		};

		ret = metacallhv(handle, "call_test", args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)1275.0);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);

		ret = metacallhv_s(handle, "call_test", args, sizeof(args) / sizeof(args[0]));

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)1275.0);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char *py_scripts_s1[] = {
			"s1.py"
		};

		void *handle = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("py", py_scripts_s1, sizeof(py_scripts_s1) / sizeof(py_scripts_s1[0]), &handle));

		ASSERT_NE((void *)NULL, (void *)handle);

		void *func = metacall_handle_function(handle, "shared_in_s1_and_s2");

		ASSERT_NE((void *)NULL, (void *)func);

		void *ret = metacallhv_s(handle, "shared_in_s1_and_s2", metacall_null_args, 0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)0, (int)strcmp(metacall_value_to_string(ret), "Hello from s1"));

		metacall_value_destroy(ret);

		const char *py_scripts_s2[] = {
			"s2.py"
		};

		EXPECT_EQ((int)0, (int)metacall_load_from_file("py", py_scripts_s2, sizeof(py_scripts_s2) / sizeof(py_scripts_s2[0]), &handle));

		ASSERT_NE((void *)NULL, (void *)handle);

		func = metacall_handle_function(handle, "shared_in_s1_and_s2");

		ASSERT_NE((void *)NULL, (void *)func);

		ret = metacallhv_s(handle, "shared_in_s1_and_s2", metacall_null_args, 0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)0, (int)strcmp(metacall_value_to_string(ret), "Hello from s2"));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_PY */

	/* Print inspect information */
	{
		size_t size = 0;

		char *inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char *)NULL, (char *)inspect_str);

		EXPECT_GT((size_t)size, (size_t)0);

		std::cout << inspect_str << std::endl;

		metacall_allocator_free(allocator, inspect_str);
	}

	metacall_allocator_destroy(allocator);

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
