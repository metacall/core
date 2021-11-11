/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
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

class metacall_c_test : public testing::Test
{
protected:
};

TEST_F(metacall_c_test, DefaultConstructor)
{
	/* File */
	const char *c_scripts[] = {
		"compiled.c"
	};

	ASSERT_EQ((int)0, (int)metacall_initialize());

	EXPECT_EQ((int)0, (int)metacall_load_from_file("c", c_scripts, sizeof(c_scripts) / sizeof(c_scripts[0]), NULL));

	// TODO
	// void *ret = metacall("compiled_sum", 3, 4);

	// EXPECT_NE((void *)NULL, (void *)ret);

	// EXPECT_EQ((long)metacall_value_to_long(ret), (long)0);

	// metacall_value_destroy(ret);

	/* Memory */
	const char c_buffer[] = {
		"int compiled_mult(int a, int b) { return a * b; }"
	};

	EXPECT_EQ((int)0, (int)metacall_load_from_memory("c", c_buffer, sizeof(c_buffer), NULL));

	// TODO
	// void *ret = metacall("compiled_mult", 3, 4);

	// EXPECT_NE((void *)NULL, (void *)ret);

	// EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

	// metacall_value_destroy(ret);

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
