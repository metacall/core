/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

#include <metacall_load_configuration_relative_test/metacall_load_configuration_relative_test.h>

#include <gtest/gtest.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

class metacall_load_configuration_relative_test : public testing::Test
{
public:
};

TEST_F(metacall_load_configuration_relative_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	void *config_allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	ASSERT_NE((void *)NULL, (void *)config_allocator);

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const enum metacall_value_id hello_boy_double_ids[] = {
			METACALL_DOUBLE, METACALL_DOUBLE
		};

		void *ret = NULL;

		ASSERT_EQ((int)0, (int)metacall_load_from_configuration(RELATIVE_CONFIGURATION_PATH "metacall_load_from_configuration_relative_node_test.json", NULL, config_allocator));

		ret = metacallt("hello_boy", hello_boy_double_ids, 3.0, 4.0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)7.0);

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	metacall_allocator_destroy(config_allocator);

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
