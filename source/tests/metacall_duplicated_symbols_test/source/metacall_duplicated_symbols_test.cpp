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

#include <gmock/gmock.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

class metacall_duplicated_symbols_test : public testing::Test
{
public:
};

TEST_F(metacall_duplicated_symbols_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Ruby */
	#if defined(OPTION_BUILD_LOADERS_RB)
	{
		const char * rb_ducktype_scripts[] =
		{
			"ducktype.rb"
		};

		const char * rb_second_scripts[] =
		{
			"second.rb"
		};

		/* Both scripts have get_second function which should fail when trying to load */

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("rb", rb_ducktype_scripts, sizeof(rb_ducktype_scripts) / sizeof(rb_ducktype_scripts[0]), NULL));

		EXPECT_NE((int) 0, (int) metacall_load_from_file("rb", rb_second_scripts, sizeof(rb_second_scripts) / sizeof(rb_second_scripts[0]), NULL));
	}
	#endif /* OPTION_BUILD_LOADERS_RB */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
