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

class metacall_node_clear_mem_test : public testing::Test
{
public:
};

TEST_F(metacall_node_clear_mem_test, DefaultConstructor)
{
	metacall_print_info();

	metacall_log_null();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const char bufferA[] =
			"console.log('A');\n";

		void *handleA = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("node", bufferA, sizeof(bufferA), &handleA));

		EXPECT_NE((void*)NULL, (void*)handleA);

		EXPECT_EQ((int)0, (int)metacall_clear(handleA));

		const char bufferB[] =
			"console.log('B');\n";

		void *handleB = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("node", bufferB, sizeof(bufferB), &handleB));

		EXPECT_NE((void*)NULL, (void*)handleB);

		EXPECT_EQ((int)0, (int)metacall_clear(handleB));
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
