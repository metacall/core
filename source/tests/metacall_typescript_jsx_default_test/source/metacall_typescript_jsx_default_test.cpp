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
#include <metacall/metacall_value.h>

class metacall_typescript_jsx_default_test : public testing::Test
{
public:
};

TEST_F(metacall_typescript_jsx_default_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* TypeScript */
#if defined(OPTION_BUILD_LOADERS_TS)
	{
		const char *jsx_scripts[] = {
			"server.jsx"
		};

		/* Load scripts */
		EXPECT_EQ((int)1, (int)metacall_load_from_file("ts", jsx_scripts, sizeof(jsx_scripts) / sizeof(jsx_scripts[0]), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_TS */

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
