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

#include <dynlink/dynlink.h>

class metacall_dynlink_path_test : public testing::Test
{
public:
};

TEST_F(metacall_dynlink_path_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	char *path = dynlink_lib_path("metacall");

	std::string library_path(METACALL_LIBRARY_PATH);

	ASSERT_NE((char *)NULL, (char *)path);

	ASSERT_EQ((int)0, (int)library_path.rfind(path, 0));

	free(path);

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
