/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#include <gmock/gmock.h>

#include <loader/loader.h>

#include <log/log.h>

class py_loader_test : public testing::Test
{
  protected:
};

TEST_F(py_loader_test, DefaultConstructor)
{
	loader_naming_name name = "example.py";

	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	EXPECT_EQ((int) 0, loader_load_from_file(name));

	EXPECT_EQ((int) 0, loader_unload());
}
