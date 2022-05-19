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

class metacall_python_port_import_test : public testing::Test
{
public:
};

TEST_F(metacall_python_port_import_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char buffer[] =
			"import sys\n"
			"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
			"import metacall\n"
			"from ramda import equals, all\n"
			"print(all(equals(3))([3, 3, 3, 3]))\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_PY */

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
