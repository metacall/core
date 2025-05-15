/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <atomic>

std::atomic<int> success_callbacks{};

class metacall_node_python_ruby_test : public testing::Test
{
public:
};

static void *hello_world_await_ok(void *result, void *data)
{
	EXPECT_NE((void *)NULL, (void *)result);

	EXPECT_EQ((void *)NULL, (void *)data);

	EXPECT_EQ((enum metacall_value_id)metacall_value_id(result), (enum metacall_value_id)METACALL_BOOL);

	EXPECT_EQ((boolean)metacall_value_to_bool(result), (boolean)1L);

	++success_callbacks;

	return NULL;
}

static void *hello_world_await_fail(void *, void *data)
{
	int this_should_never_happen = 1;

	EXPECT_EQ((void *)NULL, (void *)data);

	EXPECT_NE((int)0, (int)this_should_never_happen);

	return NULL;
}

TEST_F(metacall_node_python_ruby_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	/* Python */
	{
		static const char buffer[] =
			"print('Hello Python')\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));
	}

	/* Ruby */
	{
		static const char buffer[] =
			"puts 'Hello Ruby'\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("rb", buffer, sizeof(buffer), NULL));
	}

	/* NodeJS */
	{
		static const char buffer[] =
			"module.exports = {\n"
			"  test: async function () {\n"
			"    try {\n"
			"      const result = fetch('https://www.google.com', { signal: AbortSignal.timeout(30000) });\n"
			"      console.log(result);\n"
			"      return true;\n"
			"    } catch (e) {\n"
			"      console.log(e);\n"
			"      return false\n"
			"    }\n"
			"  }\n"
			"};\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));

		/* Await function that returns */
		void *ret = metacall_await("test", metacall_null_args, hello_world_await_ok, hello_world_await_fail, NULL);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_FUTURE);

		metacall_value_destroy(ret);
	}

	EXPECT_EQ((int)success_callbacks, (int)1);

	metacall_destroy();
}
