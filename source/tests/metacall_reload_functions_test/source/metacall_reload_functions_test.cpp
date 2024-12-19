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

class metacall_reload_functions_test : public testing::Test
{
public:
};

TEST_F(metacall_reload_functions_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char buffer0[] =
			"def f():\n"
			"	return 5\n";

		void *handle0 = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("py", buffer0, sizeof(buffer0), &handle0));

		void *ret = metacallhv_s(handle0, "f", metacall_null_args, 0);

		EXPECT_EQ((long)5, (long)metacall_value_to_long(ret));

		metacall_value_destroy(ret);

		EXPECT_EQ((int)0, (int)metacall_clear(handle0));

		const char buffer1[] =
			"def f():\n"
			"	return 6\n";

		void *handle1 = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("py", buffer1, sizeof(buffer1), &handle1));

		ret = metacallhv_s(handle1, "f", metacall_null_args, 0);

		EXPECT_EQ((long)6, (long)metacall_value_to_long(ret));

		metacall_value_destroy(ret);

		EXPECT_EQ((int)0, (int)metacall_clear(handle1));
	}
#endif /* OPTION_BUILD_LOADERS_PY */

/* Ruby */
#if defined(OPTION_BUILD_LOADERS_RB)
	{
		const char buffer0[] =
			"def f()\n"
			"	return 5\n"
			"end\n";

		void *handle0 = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("rb", buffer0, sizeof(buffer0), &handle0));

		void *ret = metacallhv_s(handle0, "f", metacall_null_args, 0);

		EXPECT_EQ((int)5, (int)metacall_value_to_int(ret));

		metacall_value_destroy(ret);

		EXPECT_EQ((int)0, (int)metacall_clear(handle0));

		const char buffer1[] =
			"def f()\n"
			"	return 6\n"
			"end\n";

		void *handle1 = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("rb", buffer1, sizeof(buffer1), &handle1));

		ret = metacallhv_s(handle1, "f", metacall_null_args, 0);

		EXPECT_EQ((int)6, (int)metacall_value_to_int(ret));

		metacall_value_destroy(ret);

		EXPECT_EQ((int)0, (int)metacall_clear(handle1));
	}
#endif /* OPTION_BUILD_LOADERS_RB */

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const char buffer0[] =
			"module.exports = {\n"
			"	f: () => 5,\n"
			"};\n";

		void *handle0 = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("node", buffer0, sizeof(buffer0), &handle0));

		void *ret = metacallhv_s(handle0, "f", metacall_null_args, 0);

		EXPECT_EQ((double)5.0, (double)metacall_value_to_double(ret));

		metacall_value_destroy(ret);

		EXPECT_EQ((int)0, (int)metacall_clear(handle0));

		const char buffer1[] =
			"module.exports = {\n"
			"	f: () => 6,\n"
			"};\n";

		void *handle1 = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("node", buffer1, sizeof(buffer1), &handle1));

		ret = metacallhv_s(handle1, "f", metacall_null_args, 0);

		EXPECT_EQ((double)6.0, (double)metacall_value_to_double(ret));

		metacall_value_destroy(ret);

		EXPECT_EQ((int)0, (int)metacall_clear(handle1));
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

	metacall_destroy();
}
