/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading ruby code at run-time into a process.
 *
 */

#include <gmock/gmock.h>

#include <loader/loader.h>

#include <log/log.h>

class rb_loader_test : public testing::Test
{
  protected:
};

TEST_F(rb_loader_test, DefaultConstructor)
{
	const loader_naming_path names[] =
	{
		"hello.rb"
	};

	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	EXPECT_EQ((int) 0, (int) loader_load_from_file("rb", names, sizeof(names) / sizeof(names[0]), NULL));

	EXPECT_EQ((int) 0, (int) loader_unload());
}
