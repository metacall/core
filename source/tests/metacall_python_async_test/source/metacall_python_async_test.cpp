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

class metacall_python_async_test : public testing::Test
{
public:
};

TEST_F(metacall_python_async_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	#if defined(OPTION_BUILD_LOADERS_PY)
	{
		/* TODO: Create another test using Curio library? */
		const char buffer[] =
			"import asyncio\n"
			"async def my_sleep(n):\n"
 		  	"\tloop = asyncio.get_event_loop()\n"
    		"\tfuture = loop.create_future()\n"
    		"\tloop.call_later(n, future.set_result, None)\n"
			"\tprint('before awaiting')\n"
   			"\tawait future\n"
			"\tprint('after awaiting')\n"
			"\treturn 'goodbye'\n";
			
		EXPECT_EQ((int) 0, (int) metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));

		void * args[] =
		{
			metacall_value_create_long(2L)
		};

		struct await_data_type
		{
			std::mutex m;
			std::condition_variable c;
		} await_data;

		std::unique_lock<std::mutex> lock(await_data.m);

		/* Test resolve */
		void * future = metacall_await("my_sleep", args, [](void * result, void * data) -> void * {	
			printf("Resolve C Callbackasdasdasd\n");
			struct await_data_type * await_data = static_cast<struct await_data_type *>(data);
			//std::unique_lock<std::mutex> lock(await_data->m);

			EXPECT_NE((void *) NULL, (void *) result);

			EXPECT_EQ((enum metacall_value_id) metacall_value_id(result), (enum metacall_value_id) METACALL_STRING);

			//EXPECT_EQ((long) 10, (long) metacall_value_to_long(result));

			printf("Resolve C Callback\n");

			await_data->c.notify_all();

			return NULL;
		}, [](void *, void * data) -> void * {
			struct await_data_type * await_data = static_cast<struct await_data_type *>(data);
			std::unique_lock<std::mutex> lock(await_data->m);
			
			/* If we reach here, there's a serious bug in the C code that is called by python after the task is done */
			int never_executed = 0;
			EXPECT_EQ((int) 1, (int) never_executed);

			printf("Reject C Callback\n");

			await_data->c.notify_one();

			return NULL;
		}, static_cast<void *>(&await_data));

		await_data.c.wait(lock);

		EXPECT_NE((void *) NULL, (void *) future);

		EXPECT_EQ((enum metacall_value_id) metacall_value_id(future), (enum metacall_value_id) METACALL_FUTURE);

		metacall_value_destroy(future);
#if 0
		/* Test reject */
		future = metacall_await("my_sleep_reject", args, [](void *, void *) -> void * {
			int this_should_never_be_executed = 0;

			EXPECT_EQ((int) 1, (int) this_should_never_be_executed);

			printf("Resolve C Callback\n");

			return NULL;
		}, [](void * result, void * data) -> void * {
			EXPECT_NE((void *) NULL, (void *) result);

			EXPECT_EQ((enum metacall_value_id) metacall_value_id(result), (enum metacall_value_id) METACALL_LONG);

			EXPECT_EQ((long) 10, (long) metacall_value_to_long(result));

			EXPECT_NE((void *) NULL, (void *) data);

			struct async_context * ctx = static_cast<struct async_context *>(data);

			EXPECT_EQ((int) 234, (int) ctx->value);

			printf("Reject C Callback\n");

			return metacall_value_create_double(15.0);
		}, static_cast<void *>(&ctx));

		EXPECT_NE((void *) NULL, (void *) future);

		EXPECT_EQ((enum metacall_value_id) metacall_value_id(future), (enum metacall_value_id) METACALL_FUTURE);

		metacall_value_destroy(future);
#endif
		metacall_value_destroy(args[0]);
	}
	#endif /* OPTION_BUILD_LOADERS_PY */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
