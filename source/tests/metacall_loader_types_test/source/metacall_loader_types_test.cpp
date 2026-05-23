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

#if defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char buffer[] = "x = 1\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));

		struct metacall_loader_type *types = NULL;
		size_t size = 0;

		ASSERT_EQ((int)0, (int)metacall_loader_types("py", &types, &size));
		ASSERT_GT(size, (size_t)0);
		ASSERT_NE(types, (struct metacall_loader_type *)NULL);

		for (size_t i = 0; i < size; ++i)
		{
			EXPECT_NE(types[i].name, (const char *)NULL);
			EXPECT_GE((int)types[i].id, 0);
		}

		free(types);
	}
#endif

	{
		struct metacall_loader_type *types = (struct metacall_loader_type *)0x1;
		size_t size = 123;

		EXPECT_NE((int)0, (int)metacall_loader_types(NULL, &types, &size));
		EXPECT_NE((int)0, (int)metacall_loader_types("py", NULL, &size));
		EXPECT_NE((int)0, (int)metacall_loader_types("py", &types, NULL));
	}

	{
		struct metacall_loader_type *types = NULL;
		size_t size = 0;

		EXPECT_NE((int)0, (int)metacall_loader_types("nonexistent_tag", &types, &size));
		EXPECT_EQ(types, (struct metacall_loader_type *)NULL);
		EXPECT_EQ(size, (size_t)0);
	}

	metacall_destroy();
}
