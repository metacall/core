/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_node_python_async_after_destroy_test : public testing::Test
{
public:
};

TEST_F(metacall_node_python_async_after_destroy_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* NodeJS & Python & Mock */
#if defined(OPTION_BUILD_LOADERS_NODE) && defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char buffer[] =
			/* NodeJS */
			"const { metacall, metacall_load_from_memory, metacall_inspect } = require('" METACALL_NODE_PORT_PATH "');\n"
			"metacall_load_from_memory('py', `"
			/* Python */
			"def sum(a, b):\n"
			"  return a + b\n"
			"`);\n"
			"function log(x) { console.log(x); return x; }\n"
			"setTimeout(() => { log(metacall('sum', 3, 4)) === 7 || process.exit(1) }, 2000);\n"
			"setTimeout(() => { log(metacall_inspect()) || process.exit(1) }, 4000);\n"
			"setTimeout(() => { log(metacall('sum', 3, 4)) === 7 || process.exit(1) }, 6000);\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_NODE && OPTION_BUILD_LOADERS_PY */

	/* This should be called before the setTimeout ends, so we can test if the node loader waits
	* to be destroyed until the event loop is completely cleaned, so we can call safely to python
	* even if the destroy was already emmited, for more info:
	* https://github.com/metacall/core/commit/4b61c2f3b22065472828dc7b718defbfc4ac3884
	* https://github.com/metacall/core/commit/e963515cf68e04c91ba0612227d5ef586c08aab6
	* https://github.com/metacall/core/commit/df701d779af0d2a7cb1f27e33aacf5f17ae20f4b
	* https://github.com/metacall/core/commit/1fc9c9244d7a5553861a458813d2cf1488ebe08c
	* https://github.com/metacall/core/commit/9b64ee533079fa0d543fc346fb7149d1086451f0
	* https://github.com/metacall/core/commit/22bd999c281f23aac04cea7df435a836631706da
	*/
	EXPECT_EQ((int)0, (int)metacall_destroy());
}
