/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_rpc_test : public testing::Test
{
public:
};

TEST_F(metacall_rpc_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* RPC */
#if defined(OPTION_BUILD_LOADERS_RPC)
	{
		const char *rpc_scripts[] = {
			"remote.url"
		};

		void *handle = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("rpc", rpc_scripts, sizeof(rpc_scripts) / sizeof(rpc_scripts[0]), &handle));

		/* Print inspect information */
		{
			size_t size = 0;

			struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

			void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

			char *inspect_str = metacall_inspect(&size, allocator);

			EXPECT_NE((char *)NULL, (char *)inspect_str);

			EXPECT_GT((size_t)size, (size_t)0);

			std::cout << inspect_str << std::endl;

			metacall_allocator_free(allocator, inspect_str);

			metacall_allocator_destroy(allocator);
		}

		const enum metacall_value_id divide_ids[] = {
			METACALL_FLOAT, METACALL_FLOAT
		};

		void *ret = metacallht_s(handle, "divide", divide_ids, 2, 50.0f, 10.0f);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((float)metacall_value_to_float(ret), (float)5.0f);

		metacall_value_destroy(ret);

		EXPECT_EQ((int)0, (int)metacall_clear(handle));

		static const char buffer[] = "http://localhost:6094/viferga/example/v1";

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("rpc", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_RPC */

	metacall_destroy();
}

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

static std::atomic<int> g_resolved(0);
static std::atomic<int> g_rejected(0);
static std::atomic<int> g_mismatches(0);
static double g_last_result = -1;

struct call_context
{
	int call_id;
	double expected;
};

static bool wait_for_count(std::atomic<int> &counter, int target, int timeout_ms)
{
	int waited = 0;

	while (counter.load() < target && waited < timeout_ms)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		waited += 10;
	}

	return counter.load() >= target;
}

static double extract_numeric(void *value)
{
	int type_id = metacall_value_id(value);

	switch (type_id)
	{
		case METACALL_FLOAT:
			return static_cast<double>(metacall_value_to_float(value));
		case METACALL_DOUBLE:
			return metacall_value_to_double(value);
		case METACALL_INT:
			return static_cast<double>(metacall_value_to_int(value));
		case METACALL_LONG:
			return static_cast<double>(metacall_value_to_long(value));
		case METACALL_SHORT:
			return static_cast<double>(metacall_value_to_short(value));
		default:
			std::cout << "    [WARN] Unexpected type_id=" << type_id << std::endl;
			return -1;
	}
}

static void *on_resolve(void *result, void * /*data*/)
{
	if (result != NULL)
	{
		g_last_result = extract_numeric(result);
		std::cout << "    [RESOLVE] result=" << g_last_result << std::endl;
		metacall_value_destroy(result);
	}
	else
	{
		std::cout << "    [RESOLVE] result=NULL" << std::endl;
	}

	g_resolved.fetch_add(1);
	return NULL;
}

static void *on_resolve_verified(void *result, void *data)
{
	call_context *ctx = static_cast<call_context *>(data);

	if (result != NULL)
	{
		double actual = extract_numeric(result);

		if (actual < ctx->expected - 0.01 || actual > ctx->expected + 0.01)
		{
			std::cout << "    [MISMATCH] call_id=" << ctx->call_id
					  << " expected=" << ctx->expected
					  << " got=" << actual << std::endl;
			g_mismatches.fetch_add(1);
		}

		metacall_value_destroy(result);
	}
	else
	{
		std::cout << "    [MISMATCH] call_id=" << ctx->call_id
				  << " result=NULL" << std::endl;
		g_mismatches.fetch_add(1);
	}

	g_resolved.fetch_add(1);
	return NULL;
}

static void *on_reject(void * /* error */, void * /* data */)
{
	g_rejected.fetch_add(1);
	return NULL;
}

TEST_F(metacall_rpc_test, AsyncSingleCall)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_RPC)
	{
		const char *rpc_scripts[] = { "remote.url" };
		ASSERT_EQ((int)0, (int)metacall_load_from_file("rpc", rpc_scripts, 1, NULL));

		g_resolved.store(0);
		g_rejected.store(0);
		g_last_result = -1;

		void *args[] = {
			metacall_value_create_float(50.0f),
			metacall_value_create_float(10.0f)
		};

		metacall_await_s("async_divide", args, 2, on_resolve, on_reject, NULL);

		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);

		bool reached = wait_for_count(g_resolved, 1, 5000);

		EXPECT_TRUE(reached) << "Callback should fire within 5s";
		EXPECT_EQ(g_resolved.load(), 1) << "Exactly 1 resolve";
		EXPECT_EQ(g_rejected.load(), 0) << "No rejects";
		EXPECT_NEAR(g_last_result, 5.0, 0.01) << "async_divide(50, 10) == 5";
	}
#endif

	metacall_destroy();
}

static const int RAPID_FIRE_COUNT = 20;

TEST_F(metacall_rpc_test, AsyncRapidFire)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_RPC)
	{
		const char *rpc_scripts[] = { "remote.url" };
		ASSERT_EQ((int)0, (int)metacall_load_from_file("rpc", rpc_scripts, 1, NULL));

		g_resolved.store(0);
		g_rejected.store(0);
		g_mismatches.store(0);

		call_context contexts[RAPID_FIRE_COUNT];

		for (int i = 0; i < RAPID_FIRE_COUNT; i++)
		{
			float a = static_cast<float>(i + 1);
			float b = 1.0f;

			contexts[i].call_id = i;
			contexts[i].expected = static_cast<double>(a / b);

			void *args[] = {
				metacall_value_create_float(a),
				metacall_value_create_float(b)
			};

			metacall_await_s("async_divide", args, 2,
				on_resolve_verified, on_reject, &contexts[i]);

			metacall_value_destroy(args[0]);
			metacall_value_destroy(args[1]);
		}

		bool reached = wait_for_count(g_resolved, RAPID_FIRE_COUNT, 10000);

		std::cout << "Resolved: " << g_resolved.load()
				  << "/" << RAPID_FIRE_COUNT
				  << ", Rejected: " << g_rejected.load()
				  << ", Mismatches: " << g_mismatches.load() << std::endl;

		EXPECT_TRUE(reached) << "All callbacks should fire within 10s";
		EXPECT_EQ(g_resolved.load(), RAPID_FIRE_COUNT) << "All calls should resolve";
		EXPECT_EQ(g_rejected.load(), 0) << "No rejects";
		EXPECT_EQ(g_mismatches.load(), 0) << "All results should match expected values";
	}
#endif

	metacall_destroy();
}

static const int NUM_THREADS = 4;
static const int CALLS_PER_THREAD = 10;
static const int TOTAL_CONCURRENT = NUM_THREADS * CALLS_PER_THREAD;

TEST_F(metacall_rpc_test, AsyncConcurrentProducers)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_RPC)
	{
		const char *rpc_scripts[] = { "remote.url" };
		ASSERT_EQ((int)0, (int)metacall_load_from_file("rpc", rpc_scripts, 1, NULL));

		g_resolved.store(0);
		g_rejected.store(0);
		g_mismatches.store(0);

		call_context all_contexts[TOTAL_CONCURRENT];
		std::vector<std::thread> threads;

		for (int t = 0; t < NUM_THREADS; t++)
		{
			threads.emplace_back([t, &all_contexts]() {
				for (int i = 0; i < CALLS_PER_THREAD; i++)
				{
					int idx = t * CALLS_PER_THREAD + i;
					float a = static_cast<float>(t * 100 + i + 1);
					float b = 1.0f;

					all_contexts[idx].call_id = idx;
					all_contexts[idx].expected = static_cast<double>(a / b);

					void *args[] = {
						metacall_value_create_float(a),
						metacall_value_create_float(b)
					};

					metacall_await_s("async_divide", args, 2,
						on_resolve_verified, on_reject, &all_contexts[idx]);

					metacall_value_destroy(args[0]);
					metacall_value_destroy(args[1]);
				}

				std::cout << "Thread " << t << ": dispatched "
						  << CALLS_PER_THREAD << " calls" << std::endl;
			});
		}

		for (auto &th : threads)
		{
			th.join();
		}

		bool reached = wait_for_count(g_resolved, TOTAL_CONCURRENT, 15000);

		std::cout << "Resolved: " << g_resolved.load()
				  << "/" << TOTAL_CONCURRENT
				  << ", Rejected: " << g_rejected.load()
				  << ", Mismatches: " << g_mismatches.load() << std::endl;

		EXPECT_TRUE(reached) << "All callbacks should fire within 15s";
		EXPECT_EQ(g_resolved.load(), TOTAL_CONCURRENT) << "All concurrent calls should resolve";
		EXPECT_EQ(g_rejected.load(), 0) << "No rejects";
		EXPECT_EQ(g_mismatches.load(), 0) << "All results should match expected values";
	}
#endif

	metacall_destroy();
}

TEST_F(metacall_rpc_test, AsyncMixedSyncAsync)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_RPC)
	{
		const char *rpc_scripts[] = { "remote.url" };
		ASSERT_EQ((int)0, (int)metacall_load_from_file("rpc", rpc_scripts, 1, NULL));

		g_resolved.store(0);
		g_rejected.store(0);
		g_last_result = -1;

		void *async_args[] = {
			metacall_value_create_float(100.0f),
			metacall_value_create_float(4.0f)
		};

		metacall_await_s("async_divide", async_args, 2, on_resolve, on_reject, NULL);

		metacall_value_destroy(async_args[0]);
		metacall_value_destroy(async_args[1]);

		const enum metacall_value_id divide_ids[] = { METACALL_DOUBLE, METACALL_DOUBLE };
		void *sync_ret = metacallt_s("divide", divide_ids, 2, 50.0, 10.0);

		EXPECT_NE((void *)NULL, (void *)sync_ret);
		EXPECT_NEAR(extract_numeric(sync_ret), 5.0, 0.01) << "Sync divide(50, 10) == 5";
		metacall_value_destroy(sync_ret);

		bool reached = wait_for_count(g_resolved, 1, 5000);

		EXPECT_TRUE(reached) << "Async callback should fire";
		EXPECT_NEAR(g_last_result, 25.0, 0.01) << "async_divide(100, 4) == 25";
	}
#endif

	metacall_destroy();
}


TEST_F(metacall_rpc_test, ShutdownMidTransfer)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_RPC)
	{
		const char *rpc_scripts[] = { "remote.url" };
		ASSERT_EQ((int)0, (int)metacall_load_from_file("rpc", rpc_scripts, 1, NULL));

		/* Fire several async calls — don't wait for any of them */
		for (int i = 0; i < 10; i++)
		{
			void *args[] = {
				metacall_value_create_float(static_cast<float>(i + 1)),
				metacall_value_create_float(1.0f)
			};

			metacall_await_s("async_divide", args, 2, on_resolve, on_reject, NULL);

			metacall_value_destroy(args[0]);
			metacall_value_destroy(args[1]);
		}
	}
#endif

	/* The real test is that this returns without crashing */
	metacall_destroy();
}

TEST_F(metacall_rpc_test, EmptyShutdown)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_RPC)
	{
		const char *rpc_scripts[] = { "remote.url" };
		ASSERT_EQ((int)0, (int)metacall_load_from_file("rpc", rpc_scripts, 1, NULL));

		/* Do nothing — no calls, no awaits */
	}
#endif

	/* Should return cleanly with no crash or hang */
	metacall_destroy();
}

static std::atomic<int> g_error_resolved(0);
static std::atomic<int> g_error_rejected(0);

static void *on_error_resolve(void *result, void * /*data*/)
{
	if (result != NULL)
	{
		std::cout << "    [ERROR_TEST RESOLVE] result=" << extract_numeric(result) << std::endl;
		metacall_value_destroy(result);
	}

	g_error_resolved.fetch_add(1);
	return NULL;
}

static void *on_error_reject(void *error, void * /*data*/)
{
	if (error != NULL)
	{
		std::cout << "    [ERROR_TEST REJECT] ";

		if (metacall_value_id(error) == METACALL_STRING)
		{
			std::cout << metacall_value_to_string(error);
		}

		std::cout << std::endl;
	}

	g_error_rejected.fetch_add(1);
	return NULL;
}

static const int GOOD_CALLS = 5;

TEST_F(metacall_rpc_test, ErrorUnderConcurrency)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_RPC)
	{
		const char *rpc_scripts[] = { "remote.url" };
		ASSERT_EQ((int)0, (int)metacall_load_from_file("rpc", rpc_scripts, 1, NULL));

		g_error_resolved.store(0);
		g_error_rejected.store(0);

		call_context good_contexts[GOOD_CALLS];

		for (int i = 0; i < GOOD_CALLS; i++)
		{
			float a = static_cast<float>((i + 1) * 10);
			float b = static_cast<float>(i + 1);

			good_contexts[i].call_id = i;
			good_contexts[i].expected = static_cast<double>(a / b);

			void *args[] = {
				metacall_value_create_float(a),
				metacall_value_create_float(b)
			};

			metacall_await_s("async_divide", args, 2,
				on_error_resolve, on_error_reject, &good_contexts[i]);

			metacall_value_destroy(args[0]);
			metacall_value_destroy(args[1]);
		}

		{
			void *bad_args[] = {
				metacall_value_create_int(1),
				metacall_value_create_int(2)
			};

			metacall_await_s("sum", bad_args, 2,
				on_error_resolve, on_error_reject, NULL);

			metacall_value_destroy(bad_args[0]);
			metacall_value_destroy(bad_args[1]);
		}

		int total_expected = GOOD_CALLS + 1;
		bool reached = false;
		int waited = 0;

		while (waited < 10000)
		{
			int total = g_error_resolved.load() + g_error_rejected.load();

			if (total >= total_expected)
			{
				reached = true;
				break;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			waited += 10;
		}

		std::cout << "ErrorUnderConcurrency: Resolved=" << g_error_resolved.load()
				  << ", Rejected=" << g_error_rejected.load() << std::endl;

		EXPECT_TRUE(reached) << "All callbacks (good + bad) should fire within 10s";

		EXPECT_EQ(g_error_resolved.load(), GOOD_CALLS) << "All valid calls should resolve";

		EXPECT_GE(g_error_rejected.load(), 1) << "Bad endpoint call should be rejected";
	}
#endif

	metacall_destroy();
}
