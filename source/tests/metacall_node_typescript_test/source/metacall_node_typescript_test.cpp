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

class metacall_node_typescript_test : public testing::Test
{
public:
};

TEST_F(metacall_node_typescript_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const char *node_scripts[] = {
			"nod.js"
		};

		/* Load scripts */
		EXPECT_EQ((int)0, (int)metacall_load_from_file("node", node_scripts, sizeof(node_scripts) / sizeof(node_scripts[0]), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

/* TypeScript */
#if defined(OPTION_BUILD_LOADERS_TS)
	{
		const char *ts_scripts[] = {
			"typedfunc/typedfunc.ts"
		};

		void *ret = NULL;

		/* Load scripts */
		EXPECT_EQ((int)0, (int)metacall_load_from_file("ts", ts_scripts, sizeof(ts_scripts) / sizeof(ts_scripts[0]), NULL));

		/* Test typed sum */
		ret = metacall("typed_sum", 3.0, 4.0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)7.0);

		metacall_value_destroy(ret);

		/* Test arrays */
		void *array_args[] = {
			metacall_value_create_array(NULL, 3)
		};

		void **array_value = metacall_value_to_array(array_args[0]);

		array_value[0] = metacall_value_create_double(3.0);
		array_value[1] = metacall_value_create_double(5.0);
		array_value[2] = metacall_value_create_double(7.0);

		ret = metacallv("typed_array", array_args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)15.0);

		metacall_value_destroy(ret);

		metacall_value_destroy(array_args[0]);

		/* Test records */
		void *record_args[] = {
			metacall_value_create_map(NULL, 1)
		};

		void **map_value = metacall_value_to_map(record_args[0]);

		map_value[0] = metacall_value_create_array(NULL, 2);

		void **tupla = metacall_value_to_array(map_value[0]);

		static const char key[] = "element";

		tupla[0] = metacall_value_create_string(key, sizeof(key) - 1);
		tupla[1] = metacall_value_create_double(6.0);

		ret = metacallv("object_record", record_args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)6.0);

		metacall_value_destroy(ret);

		metacall_value_destroy(record_args[0]);
	}
#endif /* OPTION_BUILD_LOADERS_TS */

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
