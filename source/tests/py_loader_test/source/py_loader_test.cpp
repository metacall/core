/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#include <gmock/gmock.h>

#include <loader/loader.h>

class py_loader_test : public testing::Test
{
  protected:
};

TEST_F(py_loader_test, DefaultConstructor)
{
	loader_naming_name name = "example.py";

	EXPECT_EQ((int) 0, loader_load(name));

	EXPECT_EQ((int) 0, loader_unload());
}
