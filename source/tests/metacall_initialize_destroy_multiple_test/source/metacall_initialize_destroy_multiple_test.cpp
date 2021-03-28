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

#include <gtest/gtest.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

class metacall_initialize_destroy_multiple_test : public testing::Test
{
public:
};

TEST_F(metacall_initialize_destroy_multiple_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Mock */
#if defined(OPTION_BUILD_LOADERS_MOCK)
	{
		static const char *mock_scripts[] = {
			"empty.mock"
		};

		static const char tag[] = "mock";

		ASSERT_EQ((int)1, (int)metacall_is_initialized(tag));

		EXPECT_EQ((int)0, (int)metacall_load_from_file(tag, mock_scripts, sizeof(mock_scripts) / sizeof(mock_scripts[0]), NULL));

		ASSERT_EQ((int)0, (int)metacall_is_initialized(tag));

		ASSERT_EQ((int)0, (int)metacall_destroy());

		ASSERT_EQ((int)1, (int)metacall_is_initialized(tag));
	}
#endif /* OPTION_BUILD_LOADERS_MOCK */

	ASSERT_EQ((int)0, (int)metacall_destroy());

	ASSERT_EQ((int)0, (int)metacall_destroy());
}
