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

class metacall_clear_test : public testing::Test
{
public:
};

TEST_F(metacall_clear_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char buffer[] =
			"#!/usr/bin/env python3\n"
			"def multmem(left: int, right: int) -> int:\n"
			"\tresult = left * right;\n"
			"\tprint(left, ' * ', right, ' = ', result);\n"
			"\treturn result;";

		static const char tag[] = "py";

		void *handle = NULL;

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), &handle));

		void *args[2] = {
			metacall_value_create_long(5),
			metacall_value_create_long(15)
		};

		void *ret = metacallhv(handle, "multmem", args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((long)75, (long)metacall_value_to_long(ret));

		metacall_value_destroy(ret);

		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);

		EXPECT_EQ((void *)NULL, (void *)metacall_function("multmem"));

		EXPECT_NE((void *)NULL, (void *)metacall_handle_function(handle, "multmem"));

		EXPECT_EQ((int)0, (int)metacall_clear(handle));

		EXPECT_EQ((void *)NULL, (void *)metacall_function("multmem"));
	}
#endif /* OPTION_BUILD_LOADERS_PY */

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		static const char script1[] = "function greet() { return 1 }\nmodule.exports = { greet }";
		static const char script2[] = "function greet() { return 2 }\nmodule.exports = { greet }";
		static const char script3[] = "function yeet() { return 3 }\nmodule.exports = { yeet }";

		void *handle1 = NULL;
		void *handle2 = NULL;

		void *ret;

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", script1, sizeof(script1), &handle1));

		ret = metacallhv(handle1, "greet", metacall_null_args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)1.0, (double)metacall_value_to_double(ret));

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", script2, sizeof(script2), &handle2));

		ret = metacallhv(handle2, "greet", metacall_null_args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)2.0, (double)metacall_value_to_double(ret));

		metacall_value_destroy(ret);

		// Now load script number 3 into handle number 2
		ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", script3, sizeof(script3), &handle2));

		ret = metacallhv(handle2, "yeet", metacall_null_args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)3.0, (double)metacall_value_to_double(ret));

		metacall_value_destroy(ret);

		EXPECT_EQ((int)0, (int)metacall_clear(handle1));
		EXPECT_EQ((int)0, (int)metacall_clear(handle2));
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	metacall_destroy();
}
