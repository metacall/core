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

class metacall_load_memory_empty_test : public testing::Test
{
public:
};

TEST_F(metacall_load_memory_empty_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	static const char buffer[] = "";

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_PY */

/* Ruby */
#if defined(OPTION_BUILD_LOADERS_RB)
	{
		ASSERT_EQ((int)0, (int)metacall_load_from_memory("rb", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_RB */

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

/* TypeScript */
#if defined(OPTION_BUILD_LOADERS_TS)
	{
		ASSERT_EQ((int)0, (int)metacall_load_from_memory("ts", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_TS */

	/* Non existent loader */
	ASSERT_EQ((int)1, (int)metacall_load_from_memory("asdfghjk", buffer, sizeof(buffer), NULL));

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
