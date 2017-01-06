/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#include <gmock/gmock.h>

#include <metacall/metacall.h>
#include <metacall/metacall-plugins.h>

#include <log/log.h>

class metacall_test : public testing::Test
{
public:
};

TEST_F(metacall_test, DefaultConstructor)
{
	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	metacall_print_info();

	/* Python */
	#if defined(OPTION_BUILD_PLUGINS_PY)
	{
		static const char buffer[] =
			"#!/usr/bin/python3.4\n"
			"def multmem(left: int, right: int) -> int:\n"
			"\tresult = left * right;\n"
			"\tprint(left, ' * ', right, ' = ', result);\n"
			"\treturn result;";

		static const char extension[] = "py";

		const long seven_multiples_limit = 10;

		long iterator;

		EXPECT_EQ((int) 0, (int) metacall_load_from_memory(extension, buffer, sizeof(buffer)));

		value ret = NULL;

		ret = metacall("multmem", 5, 15);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((long) value_to_long(ret), (long) 75);

		value_destroy(ret);

		log_write("metacall", LOG_LEVEL_DEBUG, "5's multiples dude!");

		for (iterator = 0; iterator <= seven_multiples_limit; ++iterator)
		{
			ret = metacall("multmem", 5, iterator);

			EXPECT_NE((value) NULL, (value) ret);

			EXPECT_EQ((long) value_to_long(ret), (long) (5 * iterator));

			value_destroy(ret);
		}
	}
	#endif /* OPTION_BUILD_PLUGINS_PY */

	/* Ruby */
	#if defined(OPTION_BUILD_PLUGINS_RB)
	{
		static const char buffer[] =
			"#!/usr/bin/ruby\n"
			"#def mem_multiply(left = 0, right = 0)\n"
			"def mem_multiply(left: Fixnum, right: Fixnum)\n"
			"	result = left * right\n"
			"	puts('Multiply', result, '!')\n"
			"	return result\n"
			"end";

		static const char extension[] = "rb";

		EXPECT_EQ((int) 0, (int) metacall_load_from_memory(extension, buffer, sizeof(buffer)));

		value ret = NULL;

		ret = metacall("mem_multiply", 5, 5);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((int) value_to_int(ret), (int) 25);

		value_destroy(ret);
	}
	#endif /* OPTION_BUILD_PLUGINS_RB */

	/* JavaScript V8 */
	#if defined(OPTION_BUILD_PLUGINS_JS)
	{
		static const char buffer[] =
			"#!/bin/sh\n"
			/*"':' //; exec \"$(command -v nodejs || command -v node)\" \"$0\" \"$@\"\n"*/
			"function mem_divide(a :: Number, b :: Number) :: Number {\n"
			"	return (a / b);\n"
			"}\n";

		static const char extension[] = "js";

		EXPECT_EQ((int) 0, (int) metacall_load_from_memory(extension, buffer, sizeof(buffer)));

		value ret = NULL;

		ret = metacall("mem_divide", 10.0, 5.0);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((double) value_to_double(ret), (double) 2.0);

		value_destroy(ret);
	}
	#endif /* OPTION_BUILD_PLUGINS_JS */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
