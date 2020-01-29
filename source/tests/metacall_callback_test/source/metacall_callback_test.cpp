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
#include <metacall/metacall_value.h>
#include <metacall/metacall_loaders.h>

class metacall_callback_test : public testing::Test
{
public:
};

TEST_F(metacall_callback_test, DefaultConstructor)
{
	metacall_print_info();

	metacall_log_stdio_type log_stdio = { stdout };

	ASSERT_EQ((int) 0, (int) metacall_log(METACALL_LOG_STDIO, (void *)&log_stdio));

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char * py_scripts[] =
		{
			"host.py"
		};

		void * ret = NULL;

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		ret = metacall("a");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((double) metacall_value_to_double(ret), (double) 3.0);

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_LOADERS_PY */

	/* NodeJS */
	#if defined(OPTION_BUILD_LOADERS_NODE)
	#if 0
	{
		const char * node_scripts[] =
		{
			"nod.js"
		};

		const enum metacall_value_id hello_boy_double_ids[] =
		{
			METACALL_DOUBLE, METACALL_DOUBLE
		};

		const char buffer[] =
			"function nodmem() {\n"
			"\treturn 43;\n"
			"\t}\n"
			"module.exports = { nodmem };\n";

		void * ret = NULL;

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("node", node_scripts, sizeof(node_scripts) / sizeof(node_scripts[0]), NULL));

		ret = metacallt("hello_boy", hello_boy_double_ids, 3.0, 4.0);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((double) metacall_value_to_double(ret), (double) 7.0);

		metacall_value_destroy(ret);

		ret = metacall("lambda");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((double) metacall_value_to_double(ret), (double) 15.0);

		metacall_value_destroy(ret);

		/* TODO: Implement all remaining calls for nod.js */

		ASSERT_EQ((int) 0, (int) metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));

		ret = metacall("nodmem");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((double) metacall_value_to_double(ret), (double) 43.0);

		metacall_value_destroy(ret);
	}
	#endif
	#endif /* OPTION_BUILD_LOADERS_NODE */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
