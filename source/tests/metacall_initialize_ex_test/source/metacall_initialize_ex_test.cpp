/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <gmock/gmock.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

class metacall_initialize_ex_test : public testing::Test
{
public:
};

TEST_F(metacall_initialize_ex_test, DefaultConstructor)
{
	#if defined(OPTION_BUILD_LOADERS_MOCK)
		static char loader_name[] = "mock";
	#endif /* OPTION_BUILD_LOADERS_MOCK */

	static struct metacall_initialize_configuration_type initialize_config[] =
	{
		#if defined(OPTION_BUILD_LOADERS_MOCK)
		{
			loader_name, NULL
		},
		#endif /* OPTION_BUILD_LOADERS_MOCK */

		{
			NULL, NULL
		}
	};

	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize_ex(initialize_config));

	/* Mock */
	#if defined(OPTION_BUILD_LOADERS_MOCK)
	{
		const char * mock_scripts[] =
		{
			"empty.mock"
		};

		ASSERT_EQ((int) 1, (int) metacall_is_initialized(loader_name));

		EXPECT_EQ((int) 0, (int) metacall_load_from_file(loader_name, mock_scripts, sizeof(mock_scripts) / sizeof(mock_scripts[0]), NULL));

		ASSERT_EQ((int) 0, (int) metacall_is_initialized(loader_name));
	}
	#endif /* OPTION_BUILD_LOADERS_MOCK */

	ASSERT_EQ((int) 0, (int) metacall_destroy());
}
