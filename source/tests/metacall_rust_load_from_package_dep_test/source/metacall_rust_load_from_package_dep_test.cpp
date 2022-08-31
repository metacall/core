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

class metacall_rust_load_from_mem_test : public testing::Test
{
protected:
};

TEST_F(metacall_rust_load_from_mem_test, DefaultConstructor)
{
	const char *rs_script = "debug/libmelody.rlib";

	ASSERT_EQ((int)0, (int)metacall_initialize());

	EXPECT_EQ((int)0, (int)metacall_load_from_package("rs", rs_script, NULL));
	// Test: Load from package

	{
		const char *text =
			"option of \"v\";"
			"capture major { some of<digit>; }"
			"\".\";"
			"capture minor { some of<digit>; }"
			"\".\";"
			"capture patch { some of<digit>; }";
		void *ret = metacall("compile", text);
		EXPECT_EQ((int)0, (int)strcmp(metacall_value_to_string(ret), R"(v?(?<major>\d+)\.(?<minor>\d+)\.(?<patch>\d+))"));
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
