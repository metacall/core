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

class file_loader_test : public testing::Test
{
  protected:
};

TEST_F(file_loader_test, DefaultConstructor)
{
	const loader_naming_tag tag = "file";

	const loader_naming_path names[] =
	{
		"favicon.ico"
	};

	const size_t size = sizeof(names) / sizeof(names[0]);

	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	EXPECT_EQ((int) 0, (int) loader_load_from_file(tag, names, size, NULL));

	for (size_t index = 0; index < size; ++index)
	{
		void * handle = loader_get_handle(tag, names[index]);

		EXPECT_NE((void *) NULL, (void *) handle);

		EXPECT_EQ((int) 0, (int) loader_clear(handle));
	}

	EXPECT_EQ((int) 0, (int) loader_unload());
}
