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

class metacall_node_port_c_lib_test : public testing::Test
{
public:
};

TEST_F(metacall_node_port_c_lib_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	static const char buffer[] =
		/* NodeJS */
		"const assert = require('assert');\n"
		"const { metacall_execution_path, metacall_load_from_package_export } = require('" METACALL_NODE_PORT_PATH "');\n"
		/* C Lib Paths */
		"metacall_execution_path('c', '" LIBGIT2_INCLUDE_DIR "');\n"
		"metacall_execution_path('c', '" LIBGIT2_LIBRARY_DIR "');\n"
		/* C Lib Require */
		"const git2 = metacall_load_from_package_export('c', 'git2');\n"
		"const { git_libgit2_init, git_libgit2_shutdown } = git2;\n"
		"console.log(git2);\n"
		/* C Lib Assert */
		"assert(git_libgit2_init() >= 0, 'libgit2 initialization failed');\n"
		"git_libgit2_shutdown();\n"
		"\n";

	ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));

	metacall_destroy();
}
