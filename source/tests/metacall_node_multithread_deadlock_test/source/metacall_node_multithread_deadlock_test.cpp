/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <thread>

class metacall_node_multithread_deadlock_test : public testing::Test
{
public:
};

TEST_F(metacall_node_multithread_deadlock_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	std::atomic<unsigned int> callbacks_executed(0);

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		/* Test if the loader can cause a data race */
    	auto runner = []()
    	{
			/* Running it over and over again making sure data race is attempted */
			for (size_t i = 0; i < 2000000; i++)
			{
				void *future = metacall("j");

				EXPECT_NE((void *)NULL, (void *)future);

				EXPECT_EQ((enum metacall_value_id)metacall_value_id(future), (enum metacall_value_id)METACALL_FUTURE);
				void *ret = metacall_await_future(
					metacall_value_to_future(future),
					[](void *result, void *) -> void * {
						EXPECT_NE((void *)NULL, (void *)result);

						EXPECT_EQ((enum metacall_value_id)metacall_value_id(result), (enum metacall_value_id)METACALL_DOUBLE);

						EXPECT_EQ((double)34.0, (double)metacall_value_to_double(result));

						// ++success_callbacks;

						return metacall_value_create_double(155.0);
					},
					[](void *, void *) -> void * {
						int this_should_never_be_executed = 0;

						EXPECT_EQ((int)1, (int)this_should_never_be_executed);

						return NULL;
					},
					NULL);

				metacall_value_destroy(future);
				metacall_value_destroy(ret);
			}
    	};

		std::thread t1 (runner);
    	std::thread t2 (runner);
    	std::thread t3 (runner);
    	std::thread t4 (runner);
    	std::thread t5 (runner);
    	std::thread t6 (runner);
    	std::thread t7 (runner);
    	std::thread t8 (runner);

		t1.join();
		t2.join();
    	t3.join();
    	t4.join();
    	t5.join();
    	t6.join();
    	t7.join();
    	t8.join();
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	EXPECT_EQ((int)0, (int)metacall_destroy());

	EXPECT_EQ((unsigned int)3, (unsigned int)callbacks_executed);
}
