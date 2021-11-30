/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#define METACALL_CALL_TEST_SIZE 10000

class metacall_node_await_chain_test : public testing::Test
{
public:
};

TEST_F(metacall_node_await_chain_test, DefaultConstructor)
{
	metacall_print_info();

	metacall_log_null();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		static const char buffer[] =
			"async function sleep(ms) { return new Promise(resolve => setTimeout(() => resolve(1), ms)) }\n"
			"module.exports = { sleep };\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));

		/* We will try to emulate this: */
		/* sleep(100).then(v => v * 2).then(v => v + 3); */

		void *args[] = {
			metacall_value_create_double(100)
		};

		auto resolve1 = [](void *result, void *) -> void * {
			printf("RESOLVE 1\n");
			fflush(stdout);
			return metacall_value_create_double(metacall_value_to_double(result) * 2);
		};

		auto resolve2 = [](void *result, void *) -> void * {
			printf("RESOLVE 2\n");
			fflush(stdout);
			return metacall_value_create_double(metacall_value_to_double(result) + 3);
		};

		auto resolve3 = [](void *result, void *) -> void * {
			printf("RESOLVE 3\n");
			fflush(stdout);
			EXPECT_EQ((double)5.0, (double)metacall_value_to_double(result));
			return NULL;
		};

		void *f1 = metacall_await("sleep", args, resolve1, NULL, NULL);

		metacall_value_destroy(args[0]);

		void *f2 = metacall_await_future(metacall_value_to_future(f1), resolve2, NULL, NULL);

		void *f3 = metacall_await_future(metacall_value_to_future(f2), resolve3, NULL, NULL);

		metacall_value_destroy(f1);
		metacall_value_destroy(f2);
		metacall_value_destroy(f3);
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
