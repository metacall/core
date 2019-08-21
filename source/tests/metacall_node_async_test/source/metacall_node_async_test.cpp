/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
#include <metacall/metacall_value.h>
#include <metacall/metacall_loaders.h>

class metacall_node_async_test : public testing::Test
{
public:
};

TEST_F(metacall_node_async_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* NodeJS */
	#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const char buffer[] =
			"async function f(x) {\n"
			"\tawait undefined;\n"
			"\treturn x;\n"
			"}\n"
			"module.exports = { f };\n";

		EXPECT_EQ((int) 0, (int) metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));

		void * args[] =
		{
			metacall_value_create_double(10.0)
		};

		void * future = metacall_async("f", args);

		metacall_value_destroy(args[0]);

		EXPECT_NE((void *) NULL, (void *) future);

		EXPECT_EQ((enum metacall_value_id) metacall_value_id(future), (enum metacall_value_id) METACALL_FUTURE);

		future = metacall_await(future, NULL, NULL, NULL);

		EXPECT_NE((void *) NULL, (void *) future);

		EXPECT_EQ((enum metacall_value_id) metacall_value_id(future), (enum metacall_value_id) METACALL_FUTURE);

		metacall_value_destroy(future);
	}
	#endif /* OPTION_BUILD_LOADERS_NODE */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
