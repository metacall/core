/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

#include <gmock/gmock.h>

#include <loader/loader.h>

#include <log/log.h>

class node_loader_test : public testing::Test
{
  protected:
};

TEST_F(node_loader_test, DefaultConstructor)
{
	const loader_naming_tag tag = "node";

	const loader_naming_path names[] =
	{
		"nod.js"
	};

	const loader_naming_path handle_names[] =
	{
		"nod"
	};

	const size_t size = sizeof(names) / sizeof(names[0]);

	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	EXPECT_EQ((int) 0, (int) loader_load_from_file(tag, names, size, NULL));

	/* TODO: Test load from memory */

	for (size_t index = 0; index < size; ++index)
	{
		void * handle = loader_get_handle(tag, handle_names[index]);

		EXPECT_NE((void *) NULL, (void *) handle);

		EXPECT_EQ((int) 0, (int) loader_clear(handle));
	}

	EXPECT_EQ((int) 0, (int) loader_unload());
}
