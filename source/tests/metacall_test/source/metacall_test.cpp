/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#include <gmock/gmock.h>

#include <metacall/metacall.h>

class metacall_test : public testing::Test
{
  public:
};

TEST_F(metacall_test, DefaultConstructor)
{
	metacall_print_info();

	EXPECT_EQ((unsigned int) 0, (unsigned int) 0);
	EXPECT_EQ((unsigned int) 3, (unsigned int) 3);

	/* ... */
}
