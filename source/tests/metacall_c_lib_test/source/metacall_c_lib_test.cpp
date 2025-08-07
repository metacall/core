/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
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

class metacall_c_lib_test : public testing::Test
{
protected:
};

TEST_F(metacall_c_lib_test, DefaultConstructor)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	ASSERT_EQ((int)0, (int)metacall_load_from_package("c", "loadtest", NULL));

	void *ret = metacall("call_cpp_func");

	EXPECT_NE((void *)NULL, (void *)ret);

	EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_LONG);

	EXPECT_EQ((long)metacall_value_to_long(ret), (long)323);

	metacall_value_destroy(ret);

	void *pair_list = NULL;

	void *args_init[] = {
		metacall_value_create_ptr(&pair_list),
	};

	ret = metacallv("pair_list_init", args_init);

	std::cout << "args_init: " << args_init[0] << std::endl;
	std::cout << "args_init: *(" << pair_list << ")" << std::endl;

	EXPECT_NE((void *)NULL, (void *)ret);

	EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_INT);

	EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

	metacall_value_destroy(ret);

	metacall_value_destroy(args_init[0]);

	void *args_value[] = {
		metacall_value_create_ptr(pair_list),
		metacall_value_create_int(2)
	};

	ret = metacallv("pair_list_value", args_value);

	EXPECT_NE((void *)NULL, (void *)ret);

	EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_DOUBLE);

	EXPECT_EQ((double)metacall_value_to_double(ret), (double)2.0);

	metacall_value_destroy(ret);

	metacall_value_destroy(args_value[0]);
	metacall_value_destroy(args_value[1]);

	void *args_destroy[] = {
		metacall_value_create_ptr(pair_list),
	};

	ret = metacallv("pair_list_destroy", args_destroy);

	EXPECT_NE((void *)NULL, (void *)ret);

	EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_NULL);

	metacall_value_destroy(ret);

	metacall_value_destroy(args_destroy[0]);

	metacall_destroy();
}
