/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_rs_test : public testing::Test
{
protected:
};

TEST_F(metacall_rs_test, DefaultConstructor)
{
	const char *rs_scripts[] = {
		"hello.rs"
	};

	ASSERT_EQ((int)0, (int)metacall_initialize());

	EXPECT_EQ((int)0, (int)metacall_load_from_file("rs", rs_scripts, sizeof(rs_scripts) / sizeof(rs_scripts[0]), NULL));

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
