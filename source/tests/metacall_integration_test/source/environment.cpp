/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */
#include <metacall-integration-test/environment.hpp>

#include <gmock/gmock.h>

#include <metacall/metacall.h>

void environment::SetUp()
{
	const char * py_scripts[] = { "example.py" };
	const char * cs_scripts[] = { "hello.cs" };

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	EXPECT_EQ((int) 0, (int) metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));
	EXPECT_EQ((int) 0, (int) metacall_load_from_file("cs", cs_scripts, sizeof(cs_scripts) / sizeof(cs_scripts[0]), NULL));
}

void environment::TearDown()
{
	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
