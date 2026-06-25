/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_csharp_empty_configuration_test : public testing::Test
{
public:
};

TEST_F(metacall_csharp_empty_configuration_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	static const char sum[] =
		"using System;\n"
		"namespace Scripts {\n"
		"\tpublic class Program {\n"
		"\t\tpublic static int sum(int a, int b) {\n"
		"\t\t\treturn 0;\n"
		"\t\t}\n"
		"\t}\n"
		"}\n";

	ASSERT_EQ((int)0, (int)metacall_load_from_memory("cs", sum, sizeof(sum), NULL));

	metacall_destroy();
}
