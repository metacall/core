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

class metacall_plugin_extension_local_test : public testing::Test
{
public:
};

TEST_F(metacall_plugin_extension_local_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	/* Extension */
	const char *ext_scripts[] = {
		"plugin_extension"
	};

	void *handle = NULL;

	ASSERT_EQ((int)0, (int)metacall_load_from_file("ext", ext_scripts, sizeof(ext_scripts) / sizeof(ext_scripts[0]), &handle));

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		void *ret = metacallhv_s(handle, "pluginA", metacall_null_args, 0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(ret));

		EXPECT_EQ((long)6, (long)metacall_value_to_long(ret));

		metacall_value_destroy(ret);
	}

	{
		void *ret = metacallhv_s(handle, "pluginB", metacall_null_args, 0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(ret));

		EXPECT_EQ((long)7, (long)metacall_value_to_long(ret));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_PY */

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		void *ret = metacallhv_s(handle, "pluginC", metacall_null_args, 0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_DOUBLE, (enum metacall_value_id)metacall_value_id(ret));

		EXPECT_EQ((double)8.0, (double)metacall_value_to_double(ret));

		metacall_value_destroy(ret);
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
