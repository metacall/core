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

class metacall_initialize_destroy_multiple_node_test : public testing::Test
{
public:
};

TEST_F(metacall_initialize_destroy_multiple_node_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		static const char buffer[] = {
			"console.log('a')"
		};

		static const char tag[] = "node";

		ASSERT_EQ((int)1, (int)metacall_is_initialized(tag));

		EXPECT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		ASSERT_EQ((int)0, (int)metacall_is_initialized(tag));

		metacall_destroy();

		ASSERT_EQ((int)1, (int)metacall_is_initialized(tag));
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	metacall_destroy();

	metacall_destroy();
}
