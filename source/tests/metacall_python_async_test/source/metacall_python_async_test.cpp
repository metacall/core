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

#include <gtest/gtest.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>
#include <metacall/metacall_value.h>

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

class metacall_python_async_test : public testing::Test
{
public:
};

TEST_F(metacall_python_async_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		/* TODO: Create another test using Curio library? */
		const char buffer[] =
			"import asyncio\n"
			"import threading\n"
			"import sys\n"

			"print('sync message', threading.current_thread().ident)\n"
			"sys.stdout.flush()\n"

			"async def my_async_fn(n):\n"
			"\tprint('inside my sleep async', threading.current_thread().ident, ':', n)\n"
			"\tsys.stdout.flush()\n"
			"\treturn 58\n"

			"async def my_async_fail_fn(n):\n"
			"\tprint('inside my sleep fail async', threading.current_thread().ident, ':', n)\n"
			"\tsys.stdout.flush()\n"
			"\traise Exception(15)\n";

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));

		/* Test for asyncness Python introspection */
		EXPECT_EQ((int)1, metacall_function_async(metacall_function("my_async_fn")));

		void *args[] = {
			metacall_value_create_long(2L)
		};

		std::thread::id this_id = std::this_thread::get_id();
		std::cout << "thread " << this_id << std::endl;

		/* Test resolve */
		{
			auto resolve = [](void *result, void *) -> void * {
				std::thread::id this_id = std::this_thread::get_id();
				std::cout << "thread " << this_id << std::endl;

				printf("Got into C callback at least\n");
				fflush(stdout);

				EXPECT_NE((void *)NULL, (void *)result);

				EXPECT_EQ((enum metacall_value_id)metacall_value_id(result), (enum metacall_value_id)METACALL_LONG);

				EXPECT_EQ((long)metacall_value_to_long(result), (long)58L);

				printf("Resolve C Callback\n");
				fflush(stdout);

				return NULL;
			};

			auto reject = [](void *, void *) -> void * {
				/* If we reach here, there's a serious bug in the C code that is called by python after the task is done */
				int never_executed = 0;
				EXPECT_EQ((int)1, (int)never_executed);

				printf("Reject C Callback\n");
				fflush(stdout);

				return NULL;
			};

			void *future = metacall_await("my_async_fn", args, resolve, reject, NULL);

			EXPECT_NE((void *)NULL, (void *)future);

			EXPECT_EQ((enum metacall_value_id)metacall_value_id(future), (enum metacall_value_id)METACALL_FUTURE);

			metacall_value_destroy(future);
		}

		/* Test reject */
		{
			auto resolve = [](void *, void *) -> void * {
				/* If we reach here, there's a serious bug in the C code that is called by python after the task is done */
				int never_executed = 0;
				EXPECT_EQ((int)1, (int)never_executed);

				printf("Reject C Callback\n");
				fflush(stdout);

				return NULL;
			};

			auto reject = [](void *result, void *) -> void * {
				std::thread::id this_id = std::this_thread::get_id();
				std::cout << "thread " << this_id << std::endl;

				printf("Got into C callback at least\n");
				fflush(stdout);

				EXPECT_NE((void *)NULL, (void *)result);

				EXPECT_EQ((enum metacall_value_id)metacall_value_id(result), (enum metacall_value_id)METACALL_LONG);

				EXPECT_EQ((long)metacall_value_to_long(result), (long)15L);

				printf("Resolve C Callback\n");
				fflush(stdout);

				return NULL;
			};

			void *future = metacall_await("my_async_fail_fn", args, resolve, reject, NULL);

			EXPECT_NE((void *)NULL, (void *)future);

			EXPECT_EQ((enum metacall_value_id)metacall_value_id(future), (enum metacall_value_id)METACALL_FUTURE);

			metacall_value_destroy(future);
		}

		metacall_value_destroy(args[0]);
	}
#endif /* OPTION_BUILD_LOADERS_PY */

	metacall_destroy();
}
