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

class metacall_node_python_port_mock_test : public testing::Test
{
public:
};

TEST_F(metacall_node_python_port_mock_test, DefaultConstructor)
{
	metacall_print_info();

	metacall_log_null();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* NodeJS & Python & Mock */
#if defined(OPTION_BUILD_LOADERS_NODE) && defined(OPTION_BUILD_LOADERS_PY) && defined(OPTION_BUILD_LOADERS_MOCK)
	{
		static const char buffer[] =
			/* NodeJS */
			"const { metacall, metacall_load_from_memory } = require('" METACALL_NODE_PORT_PATH "');\n"
			"metacall_load_from_memory('py', `"
			/* Python */
			"import sys\n"
			"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
			"import metacall\n"
			/* Mock */
			"from asd.mock import two_doubles\n"
			"print('........................................................')\n"
			"print(two_doubles(3.0, 6.0))\n" // This works
			"print('........................................................')\n"
			"def py_func(cb):\n"
			"	return cb(two_doubles)\n"
			"print(py_func(lambda f: f(3, 4)))\n" // This works too
			"`);\n"
			// This does not, probably the error is when converting mock_func from metacall value to napi:
			"const result = metacall('py_func', (mock_func) => mock_func(3, 4));\n"
			"console.log('Result:', result);\n"
			"if (result !== 3.1416) process.exit(1);\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_NODE && OPTION_BUILD_LOADERS_PY && OPTION_BUILD_LOADERS_MOCK */

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
