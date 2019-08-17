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
			"function sleep(ms) {\n"
			"\treturn new Promise(resolve => setTimeout(resolve, ms));\n"
			"}\n"
			"function f() {\n"
			"\t(new Promise(async (resolve) => {\n"
			"\t\tconsole.log('Inside f promise');\n"
			"\t\tawait sleep(100);\n"
			"\t\treturn resolve(20);\n"
			"\t})).then((x) => { process.stdout.write(`Result: ${x}`) || process.stdout.once('drain') })\n"
			"\t.catch((x) => { process.stdout.write(`Error: ${x}`) || process.stdout.once('drain') });\n"
			"\treturn 10;\n"
			"}\n"
			"module.exports = { f };\n";

		EXPECT_EQ((int) 0, (int) metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));

		void * ret = metacall("f");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((double) metacall_value_to_double(ret), (double) 10.0);

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_LOADERS_NODE */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
