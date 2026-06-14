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

class metacall_node_conversion_test : public testing::Test
{
public:
};

TEST_F(metacall_node_conversion_test, DefaultConstructor)
{
	metacall_print_info();

	metacall_log_null();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	{
		static const char script[] = "module.exports = { identity: x => x }";
		void *handle = NULL;
		void *ret;

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", script, sizeof(script), &handle));

		{
			void *args[1] = { metacall_value_create(METACALL_INT) };

			ret = metacallhv(handle, "identity", args);

			EXPECT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((enum metacall_value_id)METACALL_DOUBLE, (enum metacall_value_id)metacall_value_id(ret));

			metacall_value_destroy(ret);
			metacall_value_destroy(args[0]);
		}

		/* Type conversion matrix: iterate over all MetaCall types and record what Node returns */
		for (size_t id = 0; id < (size_t)METACALL_SIZE; ++id)
		{
			enum metacall_value_id type_id = (enum metacall_value_id)id;

			void *args[1] = { metacall_value_create(type_id) };

			if (args[0] == NULL)
			{
				std::cout << metacall_value_id_name(type_id) << " => SKIPPED (value creation failed)" << std::endl;
				continue;
			}

			ret = metacallhv(handle, "identity", args);

			if (ret == NULL)
			{
				std::cout << metacall_value_id_name(type_id) << " => SKIPPED (identity returned NULL, type not supported by node loader)" << std::endl;
				metacall_value_destroy(args[0]);
				continue;
			}

			std::cout << metacall_value_id_name(type_id) << " => " << metacall_value_id_name(metacall_value_id(ret)) << std::endl;

			metacall_value_destroy(ret);
			metacall_value_destroy(args[0]);
		}
	}

	metacall_destroy();
}
