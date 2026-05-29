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

#include <atomic>

std::atomic<int> success_callbacks{};

struct trampoline_regression_ctx_type
{
	std::atomic<unsigned int> resolve_callbacks{ 0 };
	std::atomic<unsigned int> reject_callbacks{ 0 };
} trampoline_ctx;

class metacall_node_async_test : public testing::Test
{
public:
};

TEST_F(metacall_node_async_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const char buffer[] =
			"const util = require('util');\n"
			"function f(x) {\n"
			"\treturn new Promise(r => console.log(`Promise f executed: ${util.inspect(r)} -> ${x}`) || r(x));\n"
			"}\n"
			"function g(x) {\n"
			"\treturn new Promise((_, r) => console.log(`Promise g executed: ${util.inspect(r)} -> ${x}`) || r(x));\n"
			"}\n"
			"function h() {\n"
			"\treturn new Promise((resolve) => resolve(34));\n"
			"}\n"
			"async function i() {\n"
			"\tthrow Error('Hi there!');\n"
			"}\n"
			"module.exports = { f, g, h, i };\n";

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));

		void *args[] = {
			metacall_value_create_double(10.0)
		};

		struct async_context
		{
			int value;
		} ctx = {
			234
		};

		/* Test resolve */
		void *future = metacall_await(
			"f",
			args,
			[](void *result, void *data) -> void * {
				EXPECT_NE((void *)NULL, (void *)result);

				EXPECT_EQ((enum metacall_value_id)metacall_value_id(result), (enum metacall_value_id)METACALL_DOUBLE);

				EXPECT_EQ((double)10.0, (double)metacall_value_to_double(result));

				EXPECT_NE((void *)NULL, (void *)data);

				struct async_context *ctx = static_cast<struct async_context *>(data);

				EXPECT_EQ((int)234, (int)ctx->value);

				printf("Resolve C Callback\n");

				return metacall_value_create_double(15.0);
			},
			[](void *, void *) -> void * {
				int this_should_never_be_executed = 0;

				EXPECT_EQ((int)1, (int)this_should_never_be_executed);

				printf("Reject C Callback\n");

				return NULL;
			},
			static_cast<void *>(&ctx));

		EXPECT_NE((void *)NULL, (void *)future);

		EXPECT_EQ((enum metacall_value_id)metacall_value_id(future), (enum metacall_value_id)METACALL_FUTURE);

		metacall_value_destroy(future);

		/* Test reject */
		future = metacall_await(
			"g",
			args,
			[](void *, void *) -> void * {
				int this_should_never_be_executed = 0;

				EXPECT_EQ((int)1, (int)this_should_never_be_executed);

				printf("Resolve C Callback\n");

				return NULL;
			},
			[](void *result, void *data) -> void * {
				EXPECT_NE((void *)NULL, (void *)result);

				EXPECT_EQ((enum metacall_value_id)metacall_value_id(result), (enum metacall_value_id)METACALL_DOUBLE);

				EXPECT_EQ((double)10.0, (double)metacall_value_to_double(result));

				EXPECT_NE((void *)NULL, (void *)data);

				struct async_context *ctx = static_cast<struct async_context *>(data);

				EXPECT_EQ((int)234, (int)ctx->value);

				printf("Reject C Callback\n");

				++success_callbacks;

				return metacall_value_create_double(15.0);
			},
			static_cast<void *>(&ctx));

		EXPECT_NE((void *)NULL, (void *)future);

		EXPECT_EQ((enum metacall_value_id)metacall_value_id(future), (enum metacall_value_id)METACALL_FUTURE);

		metacall_value_destroy(future);

		metacall_value_destroy(args[0]);

		/*  Regression #578: trampoline ownership. test */
		{
			const char trampoline_buffer[] =
				"function f_reg(x) { return new Promise(r => r(x)); }\n"
				"function g_reg(x) { return new Promise((_, r) => r(x)); }\n"
				"async function i_reg() { throw Error('Hi there!'); }\n"
				"module.exports = { f_reg, g_reg, i_reg };\n";

			EXPECT_EQ((int)0, (int)metacall_load_from_memory("node", trampoline_buffer, sizeof(trampoline_buffer), NULL));

			auto trampoline_resolve_cb = [](void *result, void *data) -> void * {
				EXPECT_NE((void *)NULL, (void *)result);

				trampoline_regression_ctx_type *ctx = static_cast<trampoline_regression_ctx_type *>(data);
				EXPECT_NE((void *)NULL, (void *)ctx);

				++ctx->resolve_callbacks;

				return NULL;
			};

			auto trampoline_reject_cb = [](void *result, void *data) -> void * {
				EXPECT_NE((void *)NULL, (void *)result);

				trampoline_regression_ctx_type *ctx = static_cast<trampoline_regression_ctx_type *>(data);
				EXPECT_NE((void *)NULL, (void *)ctx);

				++ctx->reject_callbacks;

				return NULL;
			};

			/* Await #1: resolve path — exercises node_loader_trampoline_resolve ownership */
			void *args_f_reg[] = { metacall_value_create_double(10.0) };

			void *future_f_reg = metacall_await("f_reg", args_f_reg, trampoline_resolve_cb, trampoline_reject_cb, static_cast<void *>(&trampoline_ctx));

			metacall_value_destroy(args_f_reg[0]);

			EXPECT_NE((void *)NULL, (void *)future_f_reg);
			EXPECT_EQ((enum metacall_value_id)METACALL_FUTURE, (enum metacall_value_id)metacall_value_id(future_f_reg));

			metacall_value_destroy(future_f_reg);

			/* Await #2: reject path — exercises node_loader_trampoline_reject ownership */
			void *args_g_reg[] = { metacall_value_create_double(20.0) };

			void *future_g_reg = metacall_await("g_reg", args_g_reg, trampoline_resolve_cb, trampoline_reject_cb, static_cast<void *>(&trampoline_ctx));

			metacall_value_destroy(args_g_reg[0]);

			EXPECT_NE((void *)NULL, (void *)future_g_reg);
			EXPECT_EQ((enum metacall_value_id)METACALL_FUTURE, (enum metacall_value_id)metacall_value_id(future_g_reg));

			metacall_value_destroy(future_g_reg);

			/* Await #3: async throw — exception surfaces as reject path */
			void *future_i_reg = metacall_await("i_reg", metacall_null_args, trampoline_resolve_cb, trampoline_reject_cb, static_cast<void *>(&trampoline_ctx));

			EXPECT_NE((void *)NULL, (void *)future_i_reg);
			EXPECT_EQ((enum metacall_value_id)METACALL_FUTURE, (enum metacall_value_id)metacall_value_id(future_i_reg));

			metacall_value_destroy(future_i_reg);
		}

		/* Test future */
		future = metacall("h");

		EXPECT_NE((void *)NULL, (void *)future);

		EXPECT_EQ((enum metacall_value_id)metacall_value_id(future), (enum metacall_value_id)METACALL_FUTURE);

		void *ret = metacall_await_future(
			metacall_value_to_future(future),
			[](void *result, void *) -> void * {
				EXPECT_NE((void *)NULL, (void *)result);

				EXPECT_EQ((enum metacall_value_id)metacall_value_id(result), (enum metacall_value_id)METACALL_DOUBLE);

				EXPECT_EQ((double)34.0, (double)metacall_value_to_double(result));

				++success_callbacks;

				return metacall_value_create_double(155.0);
			},
			[](void *, void *) -> void * {
				int this_should_never_be_executed = 0;

				EXPECT_EQ((int)1, (int)this_should_never_be_executed);

				return NULL;
			},
			NULL);

		metacall_value_destroy(future);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_FUTURE);

		void *last = metacall_await_future(
			metacall_value_to_future(ret),
			[](void *result, void *) -> void * {
				EXPECT_NE((void *)NULL, (void *)result);

				EXPECT_EQ((enum metacall_value_id)metacall_value_id(result), (enum metacall_value_id)METACALL_DOUBLE);

				EXPECT_EQ((double)155.0, (double)metacall_value_to_double(result));

				++success_callbacks;

				return NULL;
			},
			[](void *, void *) -> void * {
				int this_should_never_be_executed = 0;

				EXPECT_EQ((int)1, (int)this_should_never_be_executed);

				return NULL;
			},
			NULL);

		metacall_value_destroy(last);

		metacall_value_destroy(ret);

		/* Test throw exception inside async function */
		future = metacall_await(
			"i",
			metacall_null_args,
			[](void *, void *) -> void * {
				int this_should_never_be_executed = 0;

				EXPECT_EQ((int)1, (int)this_should_never_be_executed);

				printf("Resolve C Callback\n");

				return NULL;
			},
			[](void *result, void *data) -> void * {
				EXPECT_NE((void *)NULL, (void *)result);

				EXPECT_EQ((enum metacall_value_id)metacall_value_id(result), (enum metacall_value_id)METACALL_EXCEPTION);

				EXPECT_NE((void *)NULL, (void *)data);

				struct async_context *ctx = static_cast<struct async_context *>(data);

				EXPECT_EQ((int)234, (int)ctx->value);

				printf("Reject C Callback\n");

				++success_callbacks;

				return metacall_value_create_double(15.0);
			},
			static_cast<void *>(&ctx));

		EXPECT_NE((void *)NULL, (void *)future);

		EXPECT_EQ((enum metacall_value_id)metacall_value_id(future), (enum metacall_value_id)METACALL_FUTURE);

		metacall_value_destroy(future);
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	metacall_destroy();

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		/* Total amount of successful callbacks must be 4 */
		EXPECT_EQ((int)success_callbacks, (int)4);

		/* Regression #578: exactly 1 resolve and 2 rejects must have fired.
		 * Under ASAN (detect_leaks=1) this also confirms zero heap leaks from
		 * the trampoline unique_ptr fix in node_loader_trampoline.cpp. */
		EXPECT_EQ((unsigned int)1, (unsigned int)trampoline_ctx.resolve_callbacks);
		EXPECT_EQ((unsigned int)2, (unsigned int)trampoline_ctx.reject_callbacks);
	}
#endif /* OPTION_BUILD_LOADERS_NODE */
}
