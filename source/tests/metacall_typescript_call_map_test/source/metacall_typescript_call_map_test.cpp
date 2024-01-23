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

class metacall_typescript_call_map_test : public testing::Test
{
public:
};

TEST_F(metacall_typescript_call_map_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

/* TypeScript */
#if defined(OPTION_BUILD_LOADERS_TS)
	{
		const char *ts_scripts[] = {
			"typedfunc/typedfunc.ts"
		};

		/* Load scripts */
		EXPECT_EQ((int)0, (int)metacall_load_from_file("ts", ts_scripts, sizeof(ts_scripts) / sizeof(ts_scripts[0]), NULL));

		/* Test typed sum */
		static const char args_map[] = "{\"left\":10,\"right\":2}";

		void *ret = metacallfms(metacall_function("typed_sum"), args_map, sizeof(args_map), allocator);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)12.0);

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_TS */

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
