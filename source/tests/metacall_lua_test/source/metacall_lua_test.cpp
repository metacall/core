/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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
#include <metacall/metacall_loaders.h>
#include <metacall/metacall_value.h>

class metacall_lua_test : public testing::Test
{
public:
};

TEST_F(metacall_lua_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Lua */
#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Load from memory */
		{
			static const char buffer[] =
				"function luamin(left, right)\n"
				"	if (left > right) then\n"
				"		return right;\n"
				"	else\n"
				"		return left;\n"
				"	end\n"
				"end\n";

			const enum metacall_value_id min_ids[] = {
				METACALL_FLOAT, METACALL_FLOAT
			};

			ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

			void *ret = metacallt_s("luamin", min_ids, 2, 3.0f, 6.0f);

			EXPECT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((float)metacall_value_to_float(ret), (float)3.0f);

			metacall_value_destroy(ret);
		}

		/* Load from file */
		{
			const char *lua_scripts[] = {
				"max.lua"
			};

			const enum metacall_value_id max_ids[] = {
				METACALL_INT, METACALL_INT
			};

			ASSERT_EQ((int)0, (int)metacall_load_from_file(tag, lua_scripts, sizeof(lua_scripts) / sizeof(lua_scripts[0]), NULL));

			void *ret = metacallt_s("luamax", max_ids, 2, 3, 6);

			EXPECT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((int)metacall_value_to_int(ret), (int)6);

			metacall_value_destroy(ret);
		}
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	/* Print inspect information */
	{
		size_t size = 0;

		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		char *inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char *)NULL, (char *)inspect_str);

		EXPECT_GT((size_t)size, (size_t)0);

		std::cout << inspect_str << std::endl;

		metacall_allocator_free(allocator, inspect_str);

		metacall_allocator_destroy(allocator);
	}

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
