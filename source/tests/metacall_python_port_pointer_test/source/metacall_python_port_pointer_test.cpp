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

class metacall_python_port_pointer_test : public testing::Test
{
public:
};

TEST_F(metacall_python_port_pointer_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	/* Test value reference and dereference */
	void *v = metacall_value_create_int(34551);

	ASSERT_EQ((enum metacall_value_id)METACALL_INT, (enum metacall_value_id)metacall_value_id(v));

	void *ref = metacall_value_reference(v);

	ASSERT_EQ((enum metacall_value_id)METACALL_PTR, (enum metacall_value_id)metacall_value_id(ref));

	int *int_ptr = (int *)metacall_value_to_ptr(ref);

	*int_ptr += 10;

	void *result = metacall_value_dereference(ref);

	ASSERT_EQ((enum metacall_value_id)METACALL_INT, (enum metacall_value_id)metacall_value_id(result));

	ASSERT_EQ((int)34561, (int)metacall_value_to_int(result));

	ASSERT_EQ((void *)v, (void *)result);

	/* Test Python reference and dereference */
	static const char buffer[] =
		"import sys\n"
		"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
		"from metacall import metacall_load_from_package, metacall, metacall_value_create_ptr, metacall_value_reference, metacall_value_dereference\n"
		"metacall_load_from_package('c', 'loadtest')\n"

		"def test() -> int:\n"
		"	print('Test start')\n"
		"	sys.stdout.flush()\n"

		"	int_val = 324444\n"
		"	int_val_ref = metacall_value_reference(int_val)\n"

		"	print(int_val_ref)\n"
		"	sys.stdout.flush()\n"

		"	metacall('modify_int_ptr', int_val_ref)\n"
		"	int_val_deref = metacall_value_dereference(int_val_ref)\n"

		"	print(int_val, '!=', int_val_deref)\n"
		"	sys.stdout.flush()\n"

		"	return int_val_deref\n";

	// "def test() -> int:\n"
	// "	print('Test start')\n"
	// "	sys.stdout.flush()\n"
	// "	list_pair = metacall_value_create_ptr(None)\n"
	// "	list_pair_ref = metacall_value_reference(list_pair)\n"
	// "	result = metacall('pair_list_init', list_pair_ref)\n"
	// "	print(result)\n"
	// "	sys.stdout.flush()\n"
	// "	list_pair = metacall_value_dereference(list_pair_ref)\n"
	// "	result = metacall('pair_list_value', list_pair)\n"
	// "	print(result)\n"
	// "	sys.stdout.flush()\n"
	// "	return result\n";

	ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));

	void *ret = metacall("test");

	ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(ret));

	EXPECT_EQ((long)111L, (long)metacall_value_to_long(ret));

	metacall_value_destroy(ret);

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
