/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#include <gmock/gmock.h>

#include <metacall/metacall.h>

#include <reflect/value.h>

class metacall_test : public testing::Test
{
  public:
};

TEST_F(metacall_test, DefaultConstructor)
{
	metacall_print_info();

	EXPECT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	{
		const long seven_multiples_limit = 10;

		long iterator;

		value ret = NULL;

		ret = metacall("multiply", 5, 15);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((long) value_to_long(ret), (long) 75);

		value_destroy(ret);

		printf("7's multiples dude!\n");

		for (iterator = 0; iterator <= seven_multiples_limit; ++iterator)
		{
			ret = metacall("multiply", 7, iterator);

			EXPECT_NE((value) NULL, (value) ret);

			EXPECT_EQ((long) value_to_long(ret), (long) (7 * iterator));

			value_destroy(ret);
		}

		ret = metacall("divide", 64.0, 2.0);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((double) value_to_double(ret), (double) 32.0);

		value_destroy(ret);

		ret = metacall("sum", 1000, 3500);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((long) value_to_long(ret), (long) 4500);

		value_destroy(ret);

		ret = metacall("sum", 3, 4);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((long) value_to_long(ret), (long) 7);

		value_destroy(ret);

		EXPECT_EQ((value) NULL, (value) metacall("hello"));

		ret = metacall("strcat", "Hello ", "Universe");

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((int) 0, (int) strcmp(value_to_string(ret), "Hello Universe"));

		value_destroy(ret);

	}

	/* Ruby */
	{
		/*
		value ret = NULL;

		ret = metacall("say_multiply", 5, 7);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((int) value_to_int(ret), (int) 35);

		value_destroy(ret);

		EXPECT_EQ((void *) NULL, (void *) metacall("say_null"));



		ret = metacall("say_hello", "meta-programmer");

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((int) 0, (int) strcmp(value_to_string(ret), "Hello meta-programmer!"));

		value_destroy(ret);
		*/
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

	/* Mock */
	{
		value ret = NULL;

		ret = metacall("my_empty_func");

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((int) value_to_int(ret), (int) 1234);

		value_destroy(ret);

		ret = metacall("two_doubles", 3.14, 68.3);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((double) value_to_double(ret), (double) 3.1416);

		value_destroy(ret);

		ret = metacall("mixed_args", 'E', 16, 34L, 4.6, "hello");

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((char) value_to_char(ret), (char) 'A');

		value_destroy(ret);

		ret = metacall("new_args", "goodbye");

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((int) 0, (int) strcmp(value_to_string(ret), "Hello World"));

		value_destroy(ret);
	}

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
