/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#include <gmock/gmock.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

class metacall_clear_test : public testing::Test
{
public:
};

TEST_F(metacall_clear_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	#if defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char buffer[] =
			"#!/usr/bin/python3.5\n"
			"def multmem(left: int, right: int) -> int:\n"
			"\tresult = left * right;\n"
			"\tprint(left, ' * ', right, ' = ', result);\n"
			"\treturn result;";

		static const char tag[] = "py";

		void * handle = NULL;

		ASSERT_EQ((int) 0, (int) metacall_load_from_memory(tag, buffer, sizeof(buffer), &handle));

		void * ret = metacall("multmem", 5, 15);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) 75, (long) metacall_value_to_long(ret));

		metacall_value_destroy(ret);

		EXPECT_EQ((int) 0, (int) metacall_clear(handle));

		EXPECT_EQ((void *) NULL, (void *) metacall_function("multmem"));
	}
	#endif /* OPTION_BUILD_LOADERS_PY */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
