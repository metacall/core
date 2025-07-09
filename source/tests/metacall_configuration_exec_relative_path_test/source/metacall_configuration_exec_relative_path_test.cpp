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

class metacall_configuration_exec_relative_path_test : public testing::Test
{
public:
};

TEST_F(metacall_configuration_exec_relative_path_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char *py_scripts[] = {
			"main.py"
		};

		void *ret = NULL;

		ASSERT_EQ((int)0, (int)metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		ret = metacall("main");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)0, (int)strcmp(metacall_value_to_string(ret), "Python hello_world: test"));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_PY */

	metacall_destroy();
}
