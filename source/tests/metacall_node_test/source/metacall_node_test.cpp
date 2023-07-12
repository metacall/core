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

class metacall_node_test : public testing::Test
{
public:
};

TEST_F(metacall_node_test, DefaultConstructor)
{
	metacall_print_info();
	ASSERT_EQ((int)0, (int)metacall_initialize());

	char *s = NULL;
 	printf( "%c\n", s[0] );

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const char *node_scripts[] = {
			"nod.js"
		};

		const enum metacall_value_id hello_boy_double_ids[] = {
			METACALL_DOUBLE, METACALL_DOUBLE
		};

		void *ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("node", node_scripts, sizeof(node_scripts) / sizeof(node_scripts[0]), NULL));

		ret = metacallt("hello_boy", hello_boy_double_ids, 3.0, 4.0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)7.0);

		metacall_value_destroy(ret);

		// Test execution path
		EXPECT_EQ((int)0, (int)metacall_execution_path("node", METACALL_NODE_TEST_EXECUTION_PATH));

		const char *node_execution_path_scripts[] = {
			"inline.js"
		};

		EXPECT_EQ((int)0, (int)metacall_load_from_file("node", node_execution_path_scripts, sizeof(node_execution_path_scripts) / sizeof(node_execution_path_scripts[0]), NULL));

		EXPECT_NE((void *)NULL, (void *)metacall_function("inline"));

		// Test load from memory + map type
		const char buffer[] = {
			"module.exports = { test_map: (m) => m.a + m.b };"
		};

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));

		EXPECT_NE((void *)NULL, (void *)metacall_function("test_map"));

		void *args[] = {
			metacall_value_create_map(NULL, 2)
		};

		void **map_value = metacall_value_to_map(args[0]);

		map_value[0] = metacall_value_create_array(NULL, 2);

		void **tupla0 = metacall_value_to_array(map_value[0]);

		static const char key0[] = "a";

		tupla0[0] = metacall_value_create_string(key0, sizeof(key0) - 1);
		tupla0[1] = metacall_value_create_double(5.0);

		map_value[1] = metacall_value_create_array(NULL, 2);

		void **tupla1 = metacall_value_to_array(map_value[1]);

		static const char key1[] = "b";

		tupla1[0] = metacall_value_create_string(key1, sizeof(key1) - 1);
		tupla1[1] = metacall_value_create_double(7.0);

		ret = metacallv_s("test_map", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((double)12.0, (double)metacall_value_to_double(ret));

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);

		const char *node_module_scripts[] = {
			"path"
		};

		void *handle = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("node", node_module_scripts, sizeof(node_module_scripts) / sizeof(node_module_scripts[0]), &handle));

		EXPECT_NE((void *)NULL, (void *)handle);
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
