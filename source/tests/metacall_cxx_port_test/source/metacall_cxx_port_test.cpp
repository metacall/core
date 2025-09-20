/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <metacall/metacall.hpp>

using namespace metacall;

class metacall_cxx_port_test : public testing::Test
{
protected:
};

void *cxx_map_test(size_t argc, void *args[], void *data)
{
	map<std::string, float> m(args[0]);

	(void)argc;
	(void)data;

	EXPECT_EQ((float)m["hello"], (float)3.0f);
	EXPECT_EQ((float)m["world"], (float)4.0f);

	printf("hello => %f\n", m["hello"]);
	printf("world => %f\n", m["world"]);
	fflush(stdout);

	return metacall_value_create_null();
}

TEST_F(metacall_cxx_port_test, DefaultConstructor)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	{
		map<std::string, float> m = {
			{ "hello", 3.0f },
			{ "world", 4.0f }
		};

		void *args[] = {
			m.to_raw()
		};

		metacall_register("cxx_map_test", cxx_map_test, NULL, METACALL_NULL, 1, METACALL_MAP);

		void *ret = metacallv_s("cxx_map_test", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_NULL);

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

	metacall_destroy();
}
