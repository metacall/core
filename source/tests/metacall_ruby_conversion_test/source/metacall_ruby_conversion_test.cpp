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

class metacall_ruby_conversion_test : public testing::Test
{
public:
};

TEST_F(metacall_ruby_conversion_test, DefaultConstructor)
{
	metacall_print_info();

	metacall_log_null();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	{
		static const char script[] = "def identity(x); x; end";
		void *handle = NULL;

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("rb", script, sizeof(script), &handle));

		{
			void *args[1] = { metacall_value_create(METACALL_INT) };

			void *ret = metacallhv(handle, "identity", args);

			EXPECT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((enum metacall_value_id)METACALL_INT, (enum metacall_value_id)metacall_value_id(ret));

			metacall_value_destroy(ret);
			metacall_value_destroy(args[0]);
		}

		/* Test BOOL round-trip */
		{
			void *args[1] = { metacall_value_create_bool(1L) };
			void *ret = metacallhv(handle, "identity", args);
			EXPECT_NE((void *)NULL, (void *)ret);
			EXPECT_EQ((enum metacall_value_id)METACALL_BOOL, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((boolean)1L, metacall_value_to_bool(ret));
			metacall_value_destroy(ret);
			metacall_value_destroy(args[0]);
		}

		/* Test INT round-trip */
		{
			void *args[1] = { metacall_value_create_int(42) };
			void *ret = metacallhv(handle, "identity", args);
			EXPECT_NE((void *)NULL, (void *)ret);
			EXPECT_EQ((enum metacall_value_id)METACALL_INT, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((int)42, metacall_value_to_int(ret));
			metacall_value_destroy(ret);
			metacall_value_destroy(args[0]);
		}

		/* Test LONG round-trip */
		{
			void *args[1] = { metacall_value_create_long(123456789L) };
			void *ret = metacallhv(handle, "identity", args);
			EXPECT_NE((void *)NULL, (void *)ret);
			EXPECT_EQ((enum metacall_value_id)METACALL_INT, (enum metacall_value_id)metacall_value_id(ret));
			metacall_value_destroy(ret);
			metacall_value_destroy(args[0]);
		}

		/* Test DOUBLE round-trip */
		{
			void *args[1] = { metacall_value_create_double(3.14) };
			void *ret = metacallhv(handle, "identity", args);
			EXPECT_NE((void *)NULL, (void *)ret);
			EXPECT_EQ((enum metacall_value_id)METACALL_DOUBLE, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_DOUBLE_EQ((double)3.14, metacall_value_to_double(ret));
			metacall_value_destroy(ret);
			metacall_value_destroy(args[0]);
		}

		/* Test STRING round-trip */
		{
			void *args[1] = { metacall_value_create_string("hello", 5) };
			void *ret = metacallhv(handle, "identity", args);
			EXPECT_NE((void *)NULL, (void *)ret);
			EXPECT_EQ((enum metacall_value_id)METACALL_STRING, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_STREQ("hello", metacall_value_to_string(ret));
			metacall_value_destroy(ret);
			metacall_value_destroy(args[0]);
		}

		/* Test ARRAY round-trip */
		{
			void *arr[3] = {
				metacall_value_create_int(1),
				metacall_value_create_int(2),
				metacall_value_create_int(3)
			};
			void *args[1] = { metacall_value_create_array((const void **)arr, 3) };
			void *ret = metacallhv(handle, "identity", args);
			EXPECT_NE((void *)NULL, (void *)ret);
			EXPECT_EQ((enum metacall_value_id)METACALL_ARRAY, (enum metacall_value_id)metacall_value_id(ret));
			void **ret_arr = metacall_value_to_array(ret);
			EXPECT_EQ((size_t)3, metacall_value_count(ret));
			EXPECT_EQ((int)1, metacall_value_to_int(ret_arr[0]));
			EXPECT_EQ((int)2, metacall_value_to_int(ret_arr[1]));
			EXPECT_EQ((int)3, metacall_value_to_int(ret_arr[2]));
			metacall_value_destroy(ret);
			metacall_value_destroy(args[0]);
		}

		/* Test MAP round-trip */
		{
			void *pairs[2];
			void *kv0[2] = { metacall_value_create_string("key", 3), metacall_value_create_int(99) };
			void *kv1[2] = { metacall_value_create_string("lang", 4), metacall_value_create_string("ruby", 4) };
			pairs[0] = metacall_value_create_array((const void **)kv0, 2);
			pairs[1] = metacall_value_create_array((const void **)kv1, 2);
			void *args[1] = { metacall_value_create_map((const void **)pairs, 2) };
			void *ret = metacallhv(handle, "identity", args);
			EXPECT_NE((void *)NULL, (void *)ret);
			EXPECT_EQ((enum metacall_value_id)METACALL_MAP, (enum metacall_value_id)metacall_value_id(ret));
			EXPECT_EQ((size_t)2, metacall_value_count(ret));
			metacall_value_destroy(ret);
			metacall_value_destroy(args[0]);
		}

		/* Test BUFFER round-trip */
		{
			const char data[] = { 0x01, 0x02, 0x03, 0x04 };
			void *args[1] = { metacall_value_create_buffer(data, sizeof(data)) };
			void *ret = metacallhv(handle, "identity", args);
			EXPECT_NE((void *)NULL, (void *)ret);
			EXPECT_EQ((enum metacall_value_id)METACALL_BUFFER, (enum metacall_value_id)metacall_value_id(ret));
			metacall_value_destroy(ret);
			metacall_value_destroy(args[0]);
		}

		// TODO: We need to implement assertions
		for (size_t id = 0; id < METACALL_SIZE; ++id)
		{
			void *args[1] = { metacall_value_create((enum metacall_value_id)id) };

			void *ret = metacallhv(handle, "identity", args);

			EXPECT_NE((void *)NULL, (void *)ret);

			std::cout << metacall_value_id_name((enum metacall_value_id)id) << " => " << metacall_value_id_name(metacall_value_id(ret)) << std::endl;

			metacall_value_destroy(ret);
			metacall_value_destroy(args[0]);
		}
	}

	metacall_destroy();
}
