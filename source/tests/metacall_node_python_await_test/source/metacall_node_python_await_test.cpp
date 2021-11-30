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

class metacall_node_python_await_test : public testing::Test
{
public:
};

TEST_F(metacall_node_python_await_test, DefaultConstructor)
{
	metacall_print_info();

	metacall_log_null();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* NodeJS & Python & Mock */
#if defined(OPTION_BUILD_LOADERS_NODE) && defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char buffer[] =
			/* NodeJS */
			"const { metacall_await, metacall_load_from_memory, metacall_inspect } = require('" METACALL_NODE_PORT_PATH "');\n"
			"metacall_load_from_memory('py', `"
			/* Python */
			"import asyncio\n"
			"async def python_simple(n):\n"
			"  await asyncio.sleep(1)\n"
			"  return n\n"
			"`);\n"
			/* NodeJS Promise */
			"metacall_await('python_simple', 32).then((v) => {\n"
			"	console.log('RESULT:', v);\n"
			"	if (v !== 32) {\n"
			"		process.exit(1);\n"
			"	}\n"
			"}).catch((v) => {\n"
			"	console.log('ERROR:', v);\n"
			"	process.exit(2);\n"
			"});\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_NODE && OPTION_BUILD_LOADERS_PY */

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
