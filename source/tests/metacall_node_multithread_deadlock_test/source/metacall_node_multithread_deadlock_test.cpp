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

#include <atomic>
#include <thread>

std::atomic<int> success_callbacks{};
static const int call_size = 200000;
static const int thread_size = 8;

class metacall_node_multithread_deadlock_test : public testing::Test
{
public:
};

void test_await(void)
{
	for (int i = 0; i < call_size; ++i)
	{
		void *future = metacall_await(
			"f",
			metacall_null_args,
			[](void *result, void *data) -> void * {
				EXPECT_NE((void *)NULL, (void *)result);

				EXPECT_EQ((enum metacall_value_id)metacall_value_id(result), (enum metacall_value_id)METACALL_DOUBLE);

				EXPECT_EQ((double)34.0, (double)metacall_value_to_double(result));

				EXPECT_EQ((void *)NULL, (void *)data);

				++success_callbacks;

				return metacall_value_create_double(15.0);
			},
			[](void *, void *) -> void * {
				int this_should_never_be_executed = 0;

				EXPECT_EQ((int)1, (int)this_should_never_be_executed);

				return NULL;
			},
			NULL);

		EXPECT_NE((void *)NULL, (void *)future);

		EXPECT_EQ((enum metacall_value_id)metacall_value_id(future), (enum metacall_value_id)METACALL_FUTURE);

		metacall_value_destroy(future);
	}
}

void test_call(void)
{
	for (int i = 0; i < call_size; ++i)
	{
		void *result = metacallv_s("g", metacall_null_args, 0);

		EXPECT_NE((void *)NULL, (void *)result);

		EXPECT_EQ((enum metacall_value_id)metacall_value_id(result), (enum metacall_value_id)METACALL_DOUBLE);

		EXPECT_EQ((double)34.0, (double)metacall_value_to_double(result));

		metacall_value_destroy(result);

		++success_callbacks;
	}
}

TEST_F(metacall_node_multithread_deadlock_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const char buffer[] =
			"async function f() {\n"
			"\treturn 34;\n"
			"}\n"
			"function g() {\n"
			"\treturn 34;\n"
			"}\n"
			"module.exports = { f, g };\n";

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));

		std::thread threads[thread_size];

		for (int i = 0; i < thread_size; ++i)
		{
			threads[i] = std::thread(test_call);
		}

		for (int i = 0; i < thread_size; ++i)
		{
			threads[i].join();
		}
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	EXPECT_EQ((int)0, (int)metacall_destroy());

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		EXPECT_EQ((int)success_callbacks, (int)(call_size * thread_size));
	}
#endif /* OPTION_BUILD_LOADERS_NODE */
}
