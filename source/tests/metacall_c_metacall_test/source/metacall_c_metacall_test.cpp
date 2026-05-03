/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_c_metacall_test : public testing::Test
{
protected:
};

TEST_F(metacall_c_metacall_test, DefaultConstructor)
{
	using namespace metacall;

	ASSERT_EQ((int)0, (int)metacall_initialize());

	metacall::map<std::string, metacall::array> options = {
		{ "include_search_paths", metacall::array(METACALL_API_INCLUDE_DIR, METACALL_INCLUDE_DIR) },
		{ "headers", metacall::array(METACALL_INCLUDE_DIR "/metacall/metacall.h") },
		{ "libs", metacall::array(METACALL_LIBRARY) /* TODO: Right now we only support one lib */ }
	};

	void *handle = NULL;

	ASSERT_EQ((int)0, (int)metacall_load_from_package_ex("c", "metacall", &handle, options.to_raw()));

	void *ret = metacall::metacallht_s(handle, "metacall_print_info", nullptr, 0);

	ASSERT_NE((void *)NULL, (void *)ret);

	EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_STRING);

	EXPECT_EQ((int)0, strncmp(metacall_value_to_string(ret), "MetaCall", 8));

	printf("%s\n", metacall_value_to_string(ret));
	fflush(stdout);

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
