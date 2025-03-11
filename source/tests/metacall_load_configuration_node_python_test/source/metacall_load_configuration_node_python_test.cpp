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

class metacall_load_configuration_node_python_test : public testing::Test
{
public:
};

TEST_F(metacall_load_configuration_node_python_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	ASSERT_EQ((int)0, (int)metacall_execution_path("py", METACALL_PYTHON_PORT_PATH));

	static const char buffer[] =
		"const { metacall_load_from_configuration_export } = require('" METACALL_NODE_PORT_PATH "');\n"
		"const config = metacall_load_from_configuration_export('" METACALL_TEST_CONFIG_PATH "');\n"
		"module.exports = { enc: (v) => config.encrypt(v) }\n";

	static const char tag[] = "node";

	ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

	const enum metacall_value_id node_memory_enc_ids[] = {
		METACALL_DOUBLE
	};

	void *ret = metacallt("enc", node_memory_enc_ids, 15.0);

	EXPECT_NE((void *)NULL, (void *)ret);

	EXPECT_EQ((double)5.0, (double)metacall_value_to_double(ret));

	metacall_destroy();
}
