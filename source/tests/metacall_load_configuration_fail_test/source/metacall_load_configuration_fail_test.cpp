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

class metacall_load_configuration_fail_test : public testing::Test
{
public:
};

TEST_F(metacall_load_configuration_fail_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
	#define CONFIG_PATH(path) METACALL_TEST_CONFIG_PATH path

		/* Script with config */
		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void *config_allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		ASSERT_NE((void *)NULL, (void *)config_allocator);

		ASSERT_EQ((int)1, (int)metacall_load_from_configuration(CONFIG_PATH("metacall-wrong-language-id.json"), NULL, config_allocator));

		ASSERT_EQ((int)1, (int)metacall_load_from_configuration(CONFIG_PATH("metacall-wrong-path.json"), NULL, config_allocator));

		ASSERT_EQ((int)1, (int)metacall_load_from_configuration(CONFIG_PATH("metacall-wrong-scripts.json"), NULL, config_allocator));

		ASSERT_EQ((int)1, (int)metacall_load_from_configuration(CONFIG_PATH("metacall-empty-scripts.json"), NULL, config_allocator));

		ASSERT_EQ((int)1, (int)metacall_load_from_configuration(CONFIG_PATH("metacall-without-language-id.json"), NULL, config_allocator));

		ASSERT_EQ((int)1, (int)metacall_load_from_configuration(CONFIG_PATH("metacall-without-scripts.json"), NULL, config_allocator));

		ASSERT_EQ((int)1, (int)metacall_load_from_configuration(CONFIG_PATH("metacall-empty-object.json"), NULL, config_allocator));

		ASSERT_EQ((int)1, (int)metacall_load_from_configuration(CONFIG_PATH("metacall-empty.json"), NULL, config_allocator));

		ASSERT_EQ((int)1, (int)metacall_load_from_configuration(CONFIG_PATH("metacall-wrong-scripts-type.json"), NULL, config_allocator));

		ASSERT_EQ((int)1, (int)metacall_load_from_configuration(CONFIG_PATH("metacall-wrong-path-type.json"), NULL, config_allocator));

		ASSERT_EQ((int)1, (int)metacall_load_from_configuration(CONFIG_PATH("metacall-wrong-language-id-type.json"), NULL, config_allocator));

		metacall_allocator_destroy(config_allocator);
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
