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

class metacall_duplicated_handle_test : public testing::Test
{
public:
};

TEST_F(metacall_duplicated_handle_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char *py_scripts[] = {
			"example.py"
		};

		ASSERT_EQ((int)0, (int)metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		ASSERT_NE((int)0, (int)metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_PY */

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const char *node_scripts_0[] = {
			"duplicated.js"
		};

		const char *node_scripts_1[] = {
			"duplicated-in-subfolder/duplicated.js"
		};

		const enum metacall_value_id double_id[] = {
			METACALL_DOUBLE
		};

		/* First script */
		ASSERT_EQ((int)0, (int)metacall_load_from_file("node", node_scripts_0, sizeof(node_scripts_0) / sizeof(node_scripts_0[0]), NULL));

		void *ret = metacallt("three_times", double_id, 3.0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)9.0);

		metacall_value_destroy(ret);

		/* Second script */
		ASSERT_EQ((int)0, (int)metacall_load_from_file("node", node_scripts_1, sizeof(node_scripts_1) / sizeof(node_scripts_1[0]), NULL));

		ret = metacallt("two_times", double_id, 3.0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)6.0);

		metacall_value_destroy(ret);

		/* Script with config */
		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void *config_allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		ASSERT_NE((void *)NULL, (void *)config_allocator);

		ASSERT_EQ((int)0, (int)metacall_load_from_configuration(METACALL_TEST_CONFIG_PATH, NULL, config_allocator));

		ret = metacallt("one_time", double_id, 3.0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)3.0);

		metacall_value_destroy(ret);

		metacall_allocator_destroy(config_allocator);
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
