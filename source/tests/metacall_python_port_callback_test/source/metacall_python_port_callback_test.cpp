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

class metacall_python_port_callback_test : public testing::Test
{
public:
};

TEST_F(metacall_python_port_callback_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	static const char buffer[] =
		"import sys\n"
		"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
		"from metacall import metacall_load_from_file, metacall\n"
		"def sum_cb(a: int, b: int) -> int:\n"
		"	return a + b\n"
		"def v_cb():\n"
		"	print('hello world')\n"
		"def str_cb() -> str:\n"
		"	return 'hello world'\n"
		"def test() -> int:\n"
		"	metacall_load_from_file('c', ['cbks.c'])\n"
		"	metacall('c_void_callback', v_cb)\n"
		"	return metacall('c_long_callback', sum_cb)\n";

	// TODO:
	// "	s = metacall('c_str_callback', str_cb)\n"
	// "	print('----------------------')\n"
	// "	print(s)\n"
	// "	print('----------------------')\n"
	// "	return metacall('c_long_callback', sum_cb) + len(s)\n";

	ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));

	void *ret = metacall("test");

	ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(ret));

	EXPECT_EQ((long)7L, (long)metacall_value_to_long(ret));

	metacall_value_destroy(ret);

	metacall_destroy();
}
