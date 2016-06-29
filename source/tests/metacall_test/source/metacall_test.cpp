/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#include <gmock/gmock.h>

#include <metacall/metacall.h>

#define METACALL_TEST_SEVEN_MULTIPLES_LIMIT 0x0A

class metacall_test : public testing::Test
{
  public:
};

TEST_F(metacall_test, DefaultConstructor)
{
/*	int iterator;
*/
	metacall_print_info();

	EXPECT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
/*
	EXPECT_EQ((void *) NULL, (void *) metacall("multiply", 5, 15));

	printf("7's multiples dude!\n");

	for (iterator = 0; iterator <= METACALL_TEST_SEVEN_MULTIPLES_LIMIT; ++iterator)
	{
		EXPECT_EQ((void *) NULL, (void *) metacall("multiply", 7, iterator));
	}
	printf("Let's divide real numbers\n");

	EXPECT_EQ((void *) NULL, (void *) metacall("divide", 324.0, 13.4358));

	EXPECT_EQ((void *) NULL, (void *) metacall("sum", 1000, 3500));

	EXPECT_EQ((void *) NULL, (void *) metacall("hello"));
*/
	/* Ruby */

/*	EXPECT_EQ((void *) NULL, (void *) metacall("say_multiply", 5, 7));

	EXPECT_EQ((void *) NULL, (void *) metacall("say_null"));
*/
	/*
	EXPECT_EQ((void *) NULL, (void *) metacall("say_hello", "meta-programmer"));
	*/

	/* JavaScript SpiderMonkey */
/*
	EXPECT_EQ((void *) NULL, (void *) metacall("say_spider", 8, 4));
*/
	/* JavaScript V8 */

	EXPECT_EQ((void *) NULL, (void *) metacall("say_divide", 32, 4));

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
