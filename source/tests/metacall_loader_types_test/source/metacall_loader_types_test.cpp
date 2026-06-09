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

#include <stdlib.h>
#include <string.h>

class metacall_loader_types_test : public testing::Test
{
public:
};

TEST_F(metacall_loader_types_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	struct metacall_loader_type *types;
	size_t size;
	bool check_bool_type_exists = false;

	EXPECT_EQ((int)0, (int)metacall_loader_types("py", &types, &size));

	ASSERT_GT(size, (size_t)0);
	ASSERT_NE(types, (struct metacall_loader_type *)NULL);

	for (size_t i = 0; i < size; ++i)
	{
		EXPECT_NE(types[i].name, (const char *)NULL);
		EXPECT_GE((int)types[i].id, 0);

		std::string name_str(types[i].name);

		if (name_str == "bool")
		{
			check_bool_type_exists = true;
		}
	}

	ASSERT_EQ(check_bool_type_exists, true);

	free(types);

	metacall_destroy();
}
