/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_node_port_rs_test : public testing::Test
{
public:
};

TEST_F(metacall_node_port_rs_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* NodeJS & Rust */
#if defined(OPTION_BUILD_LOADERS_NODE) && defined(OPTION_BUILD_LOADERS_RS)
	{
		static const char buffer[] =
			/* NodeJS */
			"const assert = require('assert');\n"
			"require('" METACALL_NODE_PORT_PATH "');\n"
			/* Rust Require */
			"const { new_string, add_vec2, add_float, return_vec } = require('./basic.rs');\n"
			/* Rust Assert */
			"assert.strictEqual(new_string(123), 'get number 123');\n"
			"assert.strictEqual(add_vec2([1, 2, 3, 4]), 10);\n"
			"assert.strictEqual(add_float(12, 23), 35);\n"
			"assert.strictEqual(return_vec().reduce((partialSum, a) => partialSum + a, 0), 15);\n"
			"\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_NODE && OPTION_BUILD_LOADERS_RS */

	metacall_destroy();
}
