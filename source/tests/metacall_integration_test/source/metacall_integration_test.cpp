/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#include <gmock/gmock.h>

#include <loader/loader.h>

#include <metacall/metacall.h>

#include <reflect/reflect_value_type.h>

#include <log/log.h>

class metacall_integration_test : public testing::Test
{
protected:
};

TEST_F(metacall_integration_test, CsSayHello)
{
	metacall("SayHello");
}

TEST_F(metacall_integration_test, CsSayAny)
{
	metacall("Say", "Any");
}

TEST_F(metacall_integration_test, PyMultiply)
{
	value ret = NULL;

	ret = metacall("multiply", 5, 15);

	EXPECT_NE((value)NULL, (value)ret);

	EXPECT_EQ((long)value_to_long(ret), (long)75);

	value_destroy(ret);

	log_write("metacall", LOG_LEVEL_DEBUG, "7's multiples dude!");

}

TEST_F(metacall_integration_test, Sum)
{
	value ret = NULL;

	ret = metacall("Sum", 5, 10);

	EXPECT_NE((value)NULL, (value)ret);

	EXPECT_EQ((int)value_to_long(ret), (int)15);

	value_destroy(ret);
}
