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
#include <metacall/metacall_loaders.h>

class metacall_load_memory_test : public testing::Test
{
public:
};

TEST_F(metacall_load_memory_test, DefaultConstructor)
{
	metacall_print_info();

	metacall_log_stdio_type log_stdio = { stdout };

	ASSERT_EQ((int)0, (int)metacall_log(METACALL_LOG_STDIO, (void *)&log_stdio));

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

		const long seven_multiples_limit = 10;

		long iterator;

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = NULL;

		ret = metacall("multmem", 5, 15);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((long)metacall_value_to_long(ret), (long)75);

		metacall_value_destroy(ret);

		for (iterator = 0; iterator <= seven_multiples_limit; ++iterator)
		{
			ret = metacall("multmem", 5, iterator);

			EXPECT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((long)metacall_value_to_long(ret), (long)(5 * iterator));

			metacall_value_destroy(ret);
		}
	}
#endif /* OPTION_BUILD_LOADERS_PY */

/* Ruby */
#if defined(OPTION_BUILD_LOADERS_RB)
	{
		static const char buffer[] =
			"#!/usr/bin/ruby\n"
			"#def comment_line(a: Fixnum)\n"
			"#	puts('This never will be shown', a, '!')\n"
			"#	return a\n"
			"#end\n"
			"=begin\n"
			"def comment_multi_line(a: Fixnum)\n"
			"	puts('This =begin =end block never will be shown', a, '!')\n"
			"	return a\n"
			"end\n"
			"=end\n"
			"def mem_multiply(left: Fixnum, right: Fixnum)\n"
			"	result = left * right\n"
			"	puts('Multiply', result, '!')\n"
			"	return result\n"
			"end";

		static const char extension[] = "rb";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(extension, buffer, sizeof(buffer), NULL));

		void *ret = NULL;

		ret = metacall("mem_multiply", 5, 5);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)metacall_value_to_int(ret), (int)25);

		metacall_value_destroy(ret);

		ret = metacall("comment_line", 15);

		EXPECT_EQ((void *)NULL, (void *)ret);

		ret = metacall("comment_multi_line", 25);

		EXPECT_EQ((void *)NULL, (void *)ret);
	}
#endif /* OPTION_BUILD_LOADERS_RB */

/* JavaScript V8 */
#if defined(OPTION_BUILD_LOADERS_JS)
	{
		static const char buffer[] =
			"#!/usr/bin/env sh\n"
			/*"':' //; exec \"$(command -v nodejs || command -v node)\" \"$0\" \"$@\"\n"*/
			"/* function mem_comment(a :: Number) {\n"
			"	return 15;\n"
			"} */\n"
			"function mem_divide(a :: Number, b :: Number) :: Number {\n"
			"	return (a / b);\n"
			"}\n";

		static const char extension[] = "js";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(extension, buffer, sizeof(buffer), NULL));

		void *ret = NULL;

		ret = metacall("mem_divide", 10.0, 5.0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)2.0);

		metacall_value_destroy(ret);

		ret = metacall("mem_comment", 10.0);

		EXPECT_EQ((void *)NULL, (void *)ret);
	}
#endif /* OPTION_BUILD_LOADERS_JS */

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
