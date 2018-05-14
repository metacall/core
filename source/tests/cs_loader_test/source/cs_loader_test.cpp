/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#include <gmock/gmock.h>

#include <loader/loader.h>

#include <metacall/metacall.h>

#include <reflect/reflect_value_type.h>

#include <log/log.h>

class cs_loader_test : public testing::Test
{
protected:
};

TEST_F(cs_loader_test, SayHello)
{
	metacall("SayHello");
}

TEST_F(cs_loader_test, SayAny)
{
	metacall("Say", "Any");
}

TEST_F(cs_loader_test, Jump)
{
	value ret = NULL;

	ret = metacall("SuperJump");

	EXPECT_NE((value) NULL, (value) ret);

	EXPECT_EQ((int) 2, (int) value_to_long(ret));

	value_destroy(ret);
}

TEST_F(cs_loader_test, Sum)
{
	value ret = NULL;

	ret = metacall("Sum", 5, 10);

	EXPECT_NE((value) NULL, (value) ret);

	EXPECT_EQ((int) 15, (int) value_to_long(ret));

	value_destroy(ret);
}

TEST_F(cs_loader_test, Concat)
{
	value ret = NULL;

	ret = metacall("Concat", "Hello ", "World");

	EXPECT_NE((value) NULL, (value) ret);

	EXPECT_EQ((int) 0, (int) strcmp((const char *)value_to_string(ret), "Hello World"));

	value_destroy(ret);
}
