/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <metacall/metacall.h>
#include <metacall/metacall_value.h>
#include <metacall/metacall_loaders.h>

#include <mutex>
#include <condition_variable>

class metacall_node_port_test : public testing::Test
{
public:
};

TEST_F(metacall_node_port_test, DefaultConstructor)
{
	metacall_print_info();

	metacall_log_null();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* NodeJS */
	#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const char * node_scripts[] =
		{
			METACALL_NODE_PORT_TEST_PATH
		};

		ASSERT_EQ((int) 0, (int) metacall_load_from_file("node", node_scripts, sizeof(node_scripts) / sizeof(node_scripts[0]), NULL));

		struct await_data_type
		{
			std::mutex m;
			std::condition_variable c;
		} await_data;

		std::unique_lock<std::mutex> lock(await_data.m);

		void * future = metacall_await("main", metacall_null_args, [](void *, void * data) -> void * {
			struct await_data_type * await_data = static_cast<struct await_data_type *>(data);
			std::unique_lock<std::mutex> lock(await_data->m);
			/* TODO: Do something with result? */
			await_data->c.notify_one();
			return NULL;
		}, [](void *, void * data) -> void * {
			struct await_data_type * await_data = static_cast<struct await_data_type *>(data);
			std::unique_lock<std::mutex> lock(await_data->m);
			/* TODO: Do something with result? */
			await_data->c.notify_one();
			return NULL;
		}, static_cast<void *>(&await_data));

		EXPECT_NE((void *) NULL, (void *) future);

		EXPECT_EQ((enum metacall_value_id) metacall_value_id(future), (enum metacall_value_id) METACALL_FUTURE);

		metacall_value_destroy(future);

		await_data.c.wait(lock);
	}
	#endif /* OPTION_BUILD_LOADERS_NODE */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
