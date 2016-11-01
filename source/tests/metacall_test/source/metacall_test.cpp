/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#include <gmock/gmock.h>

#include <metacall/metacall.h>

#include <reflect/reflect_value.h>

#include <log/log.h>

class metacall_test : public testing::Test
{
public:
};

TEST_F(metacall_test, DefaultConstructor)
{
	EXPECT_EQ((int)0, (int)log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	metacall_print_info();

	EXPECT_EQ((int)0, (int)metacall_initialize());

	/* C# Netcore */
	{

		EXPECT_EQ((void *)NULL, (void *)metacall("SayHello"));

	}

	/* JavaScript SpiderMonkey */
	{
		/*
		EXPECT_EQ((void *) NULL, (void *) metacall("say_spider", 8, 4));
		*/
	}

	/* JavaScript V8 */
	{
		/*
		value ret = NULL;

		ret = metacall("say_divide", 32.0, 4.0);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((double) value_to_double(ret), (double) 8.0);

		value_destroy(ret);

		ret = metacall("some_text", "abc", "def");

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((int) 0, (int) strcmp(value_to_string(ret), "abcdef"));

		value_destroy(ret);
		*/
	}

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
