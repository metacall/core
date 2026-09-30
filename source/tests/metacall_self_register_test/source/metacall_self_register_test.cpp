/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_self_register_test : public testing::Test
{
public:
};

TEST_F(metacall_self_register_test, DefaultConstructor)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char script[] =
			"def call_print_info():\n"
			"    return metacall_print_info()\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", script, sizeof(script), NULL));

		void *ret = metacallv("call_print_info", metacall_null_args);

		ASSERT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)METACALL_STRING, (enum metacall_value_id)metacall_value_id(ret));
		EXPECT_EQ((int)0, strncmp(metacall_value_to_string(ret), "MetaCall", 8));

		metacall_value_destroy(ret);
	}
#endif

	metacall_destroy();
}
