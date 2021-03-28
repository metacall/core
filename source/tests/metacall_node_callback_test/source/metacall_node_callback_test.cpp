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

class metacall_node_callback_test : public testing::Test
{
public:
};

TEST_F(metacall_node_callback_test, DefaultConstructor)
{
	metacall_print_info();

	metacall_log_stdio_type log_stdio = { stdout };

	ASSERT_EQ((int)0, (int)metacall_log(METACALL_LOG_STDIO, (void *)&log_stdio));

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const char *node_scripts[] = {
			"host.js"
		};

		void *ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("node", node_scripts, sizeof(node_scripts) / sizeof(node_scripts[0]), NULL));

		ret = metacall("a");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)3.0);

		metacall_value_destroy(ret);

		void *handle = metacall_handle("node", "host.js");

		EXPECT_NE((void *)NULL, (void *)handle);

		EXPECT_EQ((int)0, (int)metacall_clear(handle));
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
