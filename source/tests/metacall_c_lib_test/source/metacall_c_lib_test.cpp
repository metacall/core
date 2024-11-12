/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
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

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
