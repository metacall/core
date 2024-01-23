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

class metacall_duplicated_symbols_test : public testing::Test
{
public:
};

TEST_F(metacall_duplicated_symbols_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char bufferA[] =
			"#!/usr/bin/env python3\n"
			"def multmem(left: int, right: int) -> int:\n"
			"\tresult = left * right;\n"
			"\tprint(left, ' * ', right, ' = ', result);\n"
			"\treturn result;";

		static const char bufferB[] =
			"#!/usr/bin/env python3\n"
			"def multmem(left: int, right: int) -> int:\n"
			"\tresult = left * right;\n"
			"\tprint(left, ' * ', right, ' = ', result);\n"
			"\treturn result;";

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("py", bufferA, sizeof(bufferA), NULL));

		EXPECT_EQ((int)1, (int)metacall_load_from_memory("py", bufferB, sizeof(bufferB), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_PY */

/* Ruby */
#if defined(OPTION_BUILD_LOADERS_RB)
	{
		const char *rb_ducktype_scripts[] = {
			"ducktype.rb"
		};

		const char *rb_second_scripts[] = {
			"second.rb"
		};

		/* Both scripts have get_second function which should fail when trying to load */

		EXPECT_EQ((int)0, (int)metacall_load_from_file("rb", rb_ducktype_scripts, sizeof(rb_ducktype_scripts) / sizeof(rb_ducktype_scripts[0]), NULL));

		EXPECT_EQ((int)1, (int)metacall_load_from_file("rb", rb_second_scripts, sizeof(rb_second_scripts) / sizeof(rb_second_scripts[0]), NULL));

		void *handleA = NULL;

		static const char bufferA[] =
			"def get_second(first, second)\n"
			"	puts('Second value is', second)\n"
			"	return 4\n"
			"end\n";

		void *handleB = NULL;

		static const char bufferB[] =
			"def get_second(first, second)\n"
			"	puts('Second value is', second)\n"
			"	return 6\n"
			"end\n";

		const enum metacall_value_id say_multiply_int_ids[] = {
			METACALL_INT, METACALL_INT
		};

		/* Test global scope get_second which belongs to ducktype.rb */

		void *ret = metacallt("get_second", say_multiply_int_ids, 5, 7);

		EXPECT_EQ((int)7, (int)metacall_value_to_int(ret));

		metacall_value_destroy(ret);

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("rb", bufferA, sizeof(bufferA), &handleA));

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("rb", bufferB, sizeof(bufferB), &handleB));

		void *args[2] = {
			metacall_value_create_int(234),
			metacall_value_create_int(432)
		};

		/* Test handleA get_second which belongs to bufferA */

		ret = metacallhv_s(handleA, "get_second", args, 2);

		EXPECT_EQ((int)4, (int)metacall_value_to_int(ret));

		metacall_value_destroy(ret);

		/* Test handleB get_second which belongs to bufferB */

		ret = metacallhv_s(handleB, "get_second", args, 2);

		EXPECT_EQ((int)6, (int)metacall_value_to_int(ret));

		metacall_value_destroy(ret);

		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);
	}
#endif /* OPTION_BUILD_LOADERS_RB */

/* Python + Ruby */
#if defined(OPTION_BUILD_LOADERS_PY) && defined(OPTION_BUILD_LOADERS_RB)
	{
		/* Test duplicated symbols between languages */
		static const char bufferA[] =
			"#!/usr/bin/env python3\n"
			"def betweenlangs(left: int, right: int) -> int:\n"
			"\tresult = left * right;\n"
			"\tprint(left, ' * ', right, ' = ', result);\n"
			"\treturn result;";

		static const char bufferB[] =
			"def betweenlangs(first, second)\n"
			"	puts('Second value is', second)\n"
			"	return 6\n"
			"end\n";

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("py", bufferA, sizeof(bufferA), NULL));

		EXPECT_EQ((int)1, (int)metacall_load_from_memory("rb", bufferB, sizeof(bufferB), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_PY + OPTION_BUILD_LOADERS_RB */

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
