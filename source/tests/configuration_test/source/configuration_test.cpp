/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#include <configuration/configuration.h>

#include <environment/environment_variable.h>

#include <memory/memory.h>

#include <log/log.h>

#define CONFIGURATION_PATH "CONFIGURATION_PATH"

class configuration_test : public testing::Test
{
  public:
};

TEST_F(configuration_test, DefaultConstructor)
{
	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	const char key_value[] = "value";

	char * configuration_path = environment_variable_create(CONFIGURATION_PATH, NULL);

	memory_allocator allocator = memory_allocator_std(&std::malloc, &std::realloc, &std::free);

	ASSERT_NE((memory_allocator) NULL, (memory_allocator) allocator);

	ASSERT_EQ((int) 0, (int) configuration_initialize("rapid_json", configuration_path, allocator));

	environment_variable_destroy(configuration_path);

	/* Global */
	{
		const char name[] = "global";
		const char key_value_local[] = "value_local";

		configuration global = configuration_scope(name);

		ASSERT_NE((configuration) NULL, (configuration) global);

		value v = configuration_value(global, key_value);

		ASSERT_NE((value) NULL, (value) v);

		EXPECT_EQ((int) 12345, (int) value_to_int(v));

		v = configuration_value(global, key_value_local);

		ASSERT_NE((value) NULL, (value) v);

		EXPECT_EQ((int) 321321, (int) value_to_int(v));
	}

	/* Child A */
	{
		const char name[] = "child_a";
		const char key_value_local[] = "value_a";

		configuration child_a = configuration_scope(name);

		ASSERT_NE((configuration) NULL, (configuration) child_a);

		value v = configuration_value(child_a, key_value);

		ASSERT_NE((value) NULL, (value) v);

		EXPECT_EQ((int) 65432345, (int) value_to_int(v));

		v = configuration_value(child_a, key_value_local);

		ASSERT_NE((value) NULL, (value) v);

		EXPECT_EQ((int) 555, (int) value_to_int(v));
	}

	/* Child B */
	{
		const char name[] = "child_b";
		const char key_value_local[] = "value_b";

		configuration child_b = configuration_scope(name);

		ASSERT_NE((configuration) NULL, (configuration) child_b);

		value v = configuration_value(child_b, key_value);

		ASSERT_NE((value) NULL, (value) v);

		EXPECT_EQ((int) 54321, (int) value_to_int(v));

		v = configuration_value(child_b, key_value_local);

		ASSERT_NE((value) NULL, (value) v);

		EXPECT_EQ((int) 333, (int) value_to_int(v));
	}

	/* Child C */
	{
		const char name[] = "child_c";
		const char key_value_local[] = "value_c";

		configuration child_c = configuration_scope(name);

		ASSERT_NE((configuration) NULL, (configuration) child_c);

		value v = configuration_value(child_c, key_value);

		ASSERT_NE((value) NULL, (value) v);

		EXPECT_EQ((int) 1111, (int) value_to_int(v));

		v = configuration_value(child_c, key_value_local);

		ASSERT_NE((value) NULL, (value) v);

		EXPECT_EQ((int) 8080, (int) value_to_int(v));
	}

	/* Child D */
	{
		const char name[] = "child_d";
		const char key_value_local[] = "value_d";

		configuration child_d = configuration_scope(name);

		ASSERT_NE((configuration) NULL, (configuration) child_d);

		value v = configuration_value(child_d, key_value);

		ASSERT_NE((value) NULL, (value) v);

		EXPECT_EQ((int) 22222, (int) value_to_int(v));

		v = configuration_value(child_d, key_value_local);

		ASSERT_NE((value) NULL, (value) v);

		EXPECT_EQ((int) 999999, (int) value_to_int(v));
	}

	configuration_destroy();

	memory_allocator_destroy(allocator);
}
