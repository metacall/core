/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
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

class metacall_rust_test : public testing::Test
{
protected:
};

TEST_F(metacall_rust_test, DefaultConstructor)
{
	const char *rs_scripts[] = {
		"basic.rs"
	};

	ASSERT_EQ((int)0, (int)metacall_initialize());

	EXPECT_EQ((int)0, (int)metacall_load_from_file("rs", rs_scripts, sizeof(rs_scripts) / sizeof(rs_scripts[0]), NULL));

	{
		void *array_args[] = {
			metacall_value_create_array(NULL, 3)
		};

		void **array_value = metacall_value_to_array(array_args[0]);

		array_value[0] = metacall_value_create_int(3);
		array_value[1] = metacall_value_create_int(5);
		array_value[2] = metacall_value_create_int(7);

		void *ret = metacallv_s("add_vec2", array_args, 1);
		EXPECT_EQ((int)15, (int)metacall_value_to_int(ret));
		// void *ret = metacallv_s("add_vec", array_args, 1);
		// EXPECT_EQ((int)15, (int)metacall_value_to_int(ret));
		metacall_value_destroy(array_args[0]);
		metacall_value_destroy(ret);
	}
	{
		void *array_args[] = {
			metacall_value_create_array(NULL, 3)
		};

		void **array_value = metacall_value_to_array(array_args[0]);

		array_value[0] = metacall_value_create_float(3.0);
		array_value[1] = metacall_value_create_float(5.0);
		array_value[2] = metacall_value_create_float(7.0);

		void *ret = metacallv_s("add_float_vec", array_args, 1);
		EXPECT_EQ((float)15.0, (float)metacall_value_to_float(ret));
		metacall_value_destroy(array_args[0]);
		metacall_value_destroy(ret);
	}

	{
		void *ret = metacall("add", 5, 10);
		EXPECT_EQ((int)15, (int)metacall_value_to_int(ret));
		metacall_value_destroy(ret);
	}

	{
		void *ret = metacall("run");
		EXPECT_EQ((int)0, (int)metacall_value_to_int(ret));
		metacall_value_destroy(ret);
	}

	{
		void *ret = metacall("add_float", 5.0, 10.0);
		EXPECT_EQ((float)15.0, (float)metacall_value_to_float(ret));
		metacall_value_destroy(ret);
	}

	// {
	// 	void *ret = metacall("string_len", "Test String");
	// 	EXPECT_EQ((long)11, (long)metacall_value_to_long(ret));
	// 	ret = metacall("new_string", 123);
	// 	EXPECT_EQ((int)0, (int)strcmp(metacall_value_to_string(ret), "get number 123"));
	// 	metacall_value_destroy(ret);
	// }

	{
		// test if we can return vec
		void *ret_vec = metacall("return_vec");
		void *array_args[] = {
			ret_vec
		};
		void *ret = metacallv_s("add_vec2", array_args, 1);
		EXPECT_EQ((int)15, (int)metacall_value_to_int(ret));
		metacall_value_destroy(ret_vec);
		metacall_value_destroy(ret);
	}

	{
		void *args[] = {
			metacall_value_create_map(NULL, 2)
		};

		void **map_value = metacall_value_to_map(args[0]);

		map_value[0] = metacall_value_create_array(NULL, 2);
		void **tuple0 = metacall_value_to_array(map_value[0]);
		static const int key0 = 3;
		tuple0[0] = metacall_value_create_int(key0);
		tuple0[1] = metacall_value_create_float(5.0);

		map_value[1] = metacall_value_create_array(NULL, 2);
		void **tuple1 = metacall_value_to_array(map_value[1]);
		static const int key1 = 5;
		tuple1[0] = metacall_value_create_int(key1);
		tuple1[1] = metacall_value_create_float(10.0);

		void *ret = metacallv_s("add_map", args, 1);
		EXPECT_EQ((float)15.0, (float)metacall_value_to_float(ret));
		metacall_value_destroy(args[0]);
		metacall_value_destroy(ret);
	}

	{
		// test if we can return map
		void *ret = metacall("return_map");
		void **map_value2 = metacall_value_to_map(ret);
		void **tuple0 = metacall_value_to_array(map_value2[0]);
		EXPECT_EQ((int)metacall_value_to_int(tuple0[0]), (int)metacall_value_to_float(tuple0[1]));
		void **tuple1 = metacall_value_to_array(map_value2[1]);
		EXPECT_EQ((int)metacall_value_to_int(tuple1[0]), (int)metacall_value_to_float(tuple1[1]));
		void **tuple2 = metacall_value_to_array(map_value2[2]);
		EXPECT_EQ((int)metacall_value_to_int(tuple2[0]), (int)metacall_value_to_float(tuple2[1]));
		metacall_value_destroy(ret);
	}

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
