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

class metacall_python_dict_test : public testing::Test
{
public:
};

TEST_F(metacall_python_dict_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char *py_scripts[] = {
			"dicty.py"
		};

		EXPECT_EQ((int)0, (int)metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		void *ret = metacall("nice_dict");

		EXPECT_NE((void *)NULL, (void *)ret);

		void **dict = metacall_value_to_map(ret);
		size_t size = metacall_value_count(ret);

		for (size_t iterator = 0; iterator < size; ++iterator)
		{
			void **array = metacall_value_to_array(dict[iterator]);
			char *key = metacall_value_to_string(array[0]);

			if (strcmp(key, "asd") == 0)
			{
				EXPECT_EQ((long)123, (long)metacall_value_to_long(array[1]));
			}
			else if (strcmp(key, "hello") == 0)
			{
				EXPECT_EQ((int)0, (int)strcmp(metacall_value_to_string(array[1]), "world"));
			}
			else if (strcmp(key, "efg") == 0)
			{
				EXPECT_EQ((double)3.4, (double)metacall_value_to_double(array[1]));
			}
			else
			{
				EXPECT_NE((int)0, (int)0);
			}
		}

		metacall_value_destroy(ret);

		ret = metacall("non_supported_dict");

		EXPECT_NE((void *)NULL, (void *)ret);

		dict = metacall_value_to_map(ret);
		size = metacall_value_count(ret);

		for (size_t iterator = 0; iterator < size; ++iterator)
		{
			void **array = metacall_value_to_array(dict[iterator]);
			char *key = metacall_value_to_string(array[0]);

			if (strcmp(key, "asd") == 0)
			{
				EXPECT_EQ((long)123, (long)metacall_value_to_long(array[1]));
			}
			else
			{
				EXPECT_NE((int)0, (int)0);
			}
		}

		metacall_value_destroy(ret);

		void *args[] = {
			// "old" -> 5
			// "whatever" -> 7
			metacall_value_create_map(NULL, 2)
		};

		void **map_value = metacall_value_to_map(args[0]);

		map_value[0] = metacall_value_create_array(NULL, 2);

		void **tupla0 = metacall_value_to_array(map_value[0]);

		static const char key0[] = "old";

		tupla0[0] = metacall_value_create_string(key0, sizeof(key0) - 1);
		tupla0[1] = metacall_value_create_long(5);

		map_value[1] = metacall_value_create_array(NULL, 2);

		void **tupla1 = metacall_value_to_array(map_value[1]);

		static const char key1[] = "whatever";

		tupla1[0] = metacall_value_create_string(key1, sizeof(key1) - 1);
		tupla1[1] = metacall_value_create_long(7);

		ret = metacallv_s("with_love_for_pragma_devs", args, 1);

		metacall_value_destroy(args[0]);

		void **ret_map = metacall_value_to_map(ret);
		void **ret_pair0 = metacall_value_to_array(ret_map[0]);
		char *ret_key0 = metacall_value_to_string(ret_pair0[0]);
		long ret_value0 = metacall_value_to_long(ret_pair0[1]);

		EXPECT_EQ((int)0, (int)strcmp(ret_key0, "new"));
		EXPECT_EQ((long)5, (long)ret_value0);

		void **ret_pair1 = metacall_value_to_array(ret_map[1]);
		char *ret_key1 = metacall_value_to_string(ret_pair1[0]);
		long ret_value1 = metacall_value_to_long(ret_pair1[1]);

		EXPECT_EQ((int)0, (int)strcmp(ret_key1, "whatever"));
		EXPECT_EQ((long)7, (long)ret_value1);

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_PY */

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

	metacall_destroy();
}
