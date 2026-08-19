/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
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

#include <adt/adt_set_small.h>

#include <log/log.h>

typedef char key_str[7];

static size_t iterator_counter = 0;
static std::vector<int> order_iterator;

class adt_set_small_test : public testing::Test
{
public:
};

TEST_F(adt_set_small_test, DefaultConstructor)
{
	EXPECT_EQ((int)0, (int)log_configure("metacall",
						  log_policy_format_text(),
						  log_policy_schedule_sync(),
						  log_policy_storage_sequential(),
						  log_policy_stream_stdio(stdout)));

	/* Hash String */
	{
		static key_str key_array[] = {
			"aaaaaa", "aaaaab", "aaaaba", "aaaabb",
			"aaabaa", "aaabab", "aaabba", "aaabbb",
			"aabaaa", "aabaab", "aababa", "aababb",
			"aabbaa", "aabbab", "aabbba", "aabbbb",
			"abaaaa", "abaaab", "abaaba", "abaabb",
			"ababaa", "ababab", "ababbb", "abbaaa",
			"abbaab", "abbaba", "abbabb", "abbbaa",
			"abbbab", "abbbba", "abbbbb", "baaaaa",
			"baaaab", "baaaba", "baaabb", "baabaa",
			"baabab", "baabba", "baabbb", "babaaa",
			"babaab", "bababa", "bababb", "babbaa",
			"babbab", "babbba", "babbbb", "bbaaaa",
			"bbaaab", "bbaaba", "bbaabb", "bbabaa",
			"bbabab", "bbabba", "bbabbb", "bbbaaa",
			"bbbaab", "bbbaba", "bbbabb", "bbbbaa",
			"bbbbab", "bbbbba", "bbbbbb"
		};

		static const size_t key_array_size = sizeof(key_array) / sizeof(key_array[0]);

		static int value_array[] = {
			10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
			20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
			30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
			40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
			50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
			60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
			70, 71, 72
		};

		static const size_t value_array_size = sizeof(value_array) / sizeof(value_array[0]);

		EXPECT_EQ((size_t)key_array_size, (size_t)value_array_size);

		set_small s = set_small_create(key_array_size);

		/* Insert value */
		for (size_t i = 0; i < key_array_size; ++i)
		{
			EXPECT_EQ((int)0, (int)set_small_insert(s, key_array[i], &value_array[i]));
		}

		/* Iterators */
		iterator_counter = 0;
		struct set_small_iterator_type it;

		for (set_small_iterator_begin(&it, s); set_small_iterator_end(&it) != 0; set_small_iterator_next(&it))
		{
			char *key = (char *)set_small_iterator_key(&it);
			int *value = (int *)set_small_iterator_value(&it);

			log_write("metacall", LOG_LEVEL_DEBUG, "[%s -> %d]", (char *)key, *((int *)(value)));

			order_iterator.push_back(*((int *)(value)));

			iterator_counter++;
		}

		EXPECT_EQ((size_t)iterator_counter, (size_t)value_array_size);

		std::vector<int> expected(std::begin(value_array), std::end(value_array));

		EXPECT_EQ(expected, order_iterator);

		/* Get value */
		for (size_t i = 0; i < key_array_size; ++i)
		{
			int *value = (int *)set_small_get(s, key_array[i]);

			EXPECT_NE((int *)NULL, (int *)value);

			EXPECT_EQ((int)value_array[i], (int)*value);
		}

		/* Overwrite value */
		EXPECT_EQ((int)0, (int)set_small_insert(s, key_array[1], &value_array[0]));

		int *value = (int *)set_small_get(s, key_array[1]);

		EXPECT_EQ((int)value_array[0], (int)*value);

		EXPECT_EQ((size_t)key_array_size, (size_t)set_small_size(s));

		/* Remove value */
		for (size_t i = 0; i < key_array_size; ++i)
		{
			int *remove_value = (int *)set_small_remove(s, key_array[i]);

			if (i == 1)
			{
				EXPECT_EQ((int)value_array[0], (int)*remove_value);
			}
			else
			{
				EXPECT_EQ((int)value_array[i], (int)*remove_value);
			}
		}

		set_small_destroy(s);
	}
}
