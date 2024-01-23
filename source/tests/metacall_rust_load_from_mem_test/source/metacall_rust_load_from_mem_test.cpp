/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
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

class metacall_rust_load_from_mem_test : public testing::Test
{
protected:
};

TEST_F(metacall_rust_load_from_mem_test, DefaultConstructor)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	// Test: Load from memory
	static const char buffer[] =
		"fn add(num_1: i32, num_2: i32) -> i32 {\n"
		"\tnum_1 + num_2\n"
		"}"
		"fn add2(num_1: f32, num_2: f32) -> f32 {\n"
		"\tnum_1 + num_2\n"
		"}";

	EXPECT_EQ((int)0, (int)metacall_load_from_memory("rs", buffer, sizeof(buffer), NULL));
	void *ret = metacall("add", 5, 10);
	EXPECT_EQ((int)15, (int)metacall_value_to_int(ret));
	metacall_value_destroy(ret);
	ret = metacall("add2", 5.0, 10.0);
	EXPECT_EQ((float)15.0, (float)metacall_value_to_float(ret));
	metacall_value_destroy(ret);

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
