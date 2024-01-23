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

class metacall_python_port_import_test : public testing::Test
{
public:
};

TEST_F(metacall_python_port_import_test, metacall_node_ramda_case_1)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	/* Case 0 */
	{
		static const char buffer[] =
			"import sys\n"
			"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
			"import metacall\n"
			"from asd.mock import two_doubles\n"
			"if two_doubles(3.0, 6.0) != 3.1416:\n"
			"	sys.exit(1)\n";

		void *handle = NULL;

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), &handle));

		ASSERT_NE((void *)handle, (void *)NULL);
	}

	/* Case 1 */
	{
		static const char buffer[] =
			"import sys\n"
			"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
			"import metacall\n"
			"import metacall.node.ramda\n"
			"if metacall.node.ramda.all(metacall.node.ramda.equals(3))([3, 3, 3, 3]) != True:\n"
			"	sys.exit(1)\n";

		void *handle = NULL;

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), &handle));

		ASSERT_NE((void *)handle, (void *)NULL);
	}

	/* Case 2 */
	{
		static const char buffer[] =
			"import sys\n"
			"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
			"import metacall\n"
			"from metacall.node.ramda import equals, all\n"
			"if all(equals(3))([3, 3, 3, 3]) != True:\n"
			"	sys.exit(1)\n";

		void *handle = NULL;

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), &handle));

		ASSERT_NE((void *)handle, (void *)NULL);
	}

	/* Case 2 star */
	{
		static const char buffer[] =
			"import sys\n"
			"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
			"import metacall\n"
			"from metacall.node.ramda import *\n"
			"if all(equals(3))([3, 3, 3, 3]) != True:\n"
			"	sys.exit(1)\n";

		void *handle = NULL;

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), &handle));

		ASSERT_NE((void *)handle, (void *)NULL);
	}

	/* Case 3 */
	{
		static const char buffer[] =
			"import sys\n"
			"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
			"import metacall\n"
			"from metacall.node import ramda, path\n"
			"if ramda.all(ramda.equals(3))([3, 3, 3, 3]) != True or path.extname('index.html') != '.html':\n"
			"	sys.exit(1)\n";

		void *handle = NULL;

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), &handle));

		ASSERT_NE((void *)handle, (void *)NULL);
	}

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
