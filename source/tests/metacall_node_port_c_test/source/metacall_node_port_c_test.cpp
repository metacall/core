/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_node_port_c_test : public testing::Test
{
public:
};

TEST_F(metacall_node_port_c_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	static const char buffer[] =
		"const assert = require('assert');\n"
		"const { metacall_load_from_file_export } = require('" METACALL_NODE_PORT_PATH "');\n"
		"const { return_text, process_text } = metacall_load_from_file_export('c', ['compiled.c']);\n"
		"const result = return_text();\n"
		"console.log(`'${result}'`);\n"
		"assert(result == 'hello');\n"
		"console.log(result);\n"
		"process_text('test_test');\n"
		"\n";

	ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));

	metacall_destroy();
}
