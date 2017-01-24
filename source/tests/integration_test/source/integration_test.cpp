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

#include <log/log.h>

class integration_test : public testing::Test
{
protected:
};

TEST_F(integration_test, CsSayHello)
{
	metacall("SayHello");
}

TEST_F(integration_test, CsSayAny)
{
	metacall("Say", "Any");
}

TEST_F(integration_test, PyMultiply)
{
	value ret = NULL;

	ret = metacall("multiply", 5, 15);

	EXPECT_NE((value)NULL, (value)ret);

	EXPECT_EQ((long)value_to_long(ret), (long)75);

	value_destroy(ret);

	log_write("metacall", LOG_LEVEL_DEBUG, "7's multiples dude!");

}
