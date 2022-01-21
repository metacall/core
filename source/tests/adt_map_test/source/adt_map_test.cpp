/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <adt/adt_map.h>

#include <log/log.h>

typedef char key_str[7];

static size_t iterator_counter = 0;

int map_cb_iterate_str_to_int(map m, map_key key, map_value value, map_cb_iterate_args args)
{
	if (m && args == NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "%s -> %d", (char *)key, *((int *)(value)));

		++iterator_counter;

		return 0;
	}

	return 1;
}

class adt_map_test : public testing::Test
{
public:
};

TEST_F(adt_map_test, map_int)
{
	EXPECT_EQ((int)0, (int)log_configure("metacall",
						  log_policy_format_text(),
						  log_policy_schedule_sync(),
						  log_policy_storage_sequential(),
						  log_policy_stream_stdio(stdout)));

	map m = map_create(&hash_callback_str, &comparable_callback_str);

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

	/* Insert value */
	for (size_t i = 0; i < key_array_size; ++i)
	{
		EXPECT_EQ((int)0, (int)map_insert(m, key_array[i], &value_array[i]));
	}

	/* Insert duplicated values */
	for (size_t i = 0; i < key_array_size; ++i)
	{
		EXPECT_EQ((int)0, (int)map_insert(m, key_array[i], &value_array[value_array_size - 1 - i]));
	}

	map_iterate(m, &map_cb_iterate_str_to_int, NULL);

	EXPECT_EQ((size_t)iterator_counter, (size_t)value_array_size * 2);

	/* Get value */
	for (size_t i = 0; i < key_array_size; ++i)
	{
		vector v = map_get(m, key_array[i]);

		EXPECT_EQ((size_t)vector_size(v), (size_t)2);

		int *value0 = vector_at_type(v, 0, int *);

		EXPECT_EQ((int)(value_array[i] == *value0 || value_array[value_array_size - 1 - i] == *value0), (int)1);

		int *value1 = vector_at_type(v, 1, int *);

		EXPECT_EQ((int)(value_array[i] == *value1 || value_array[value_array_size - 1 - i] == *value1), (int)1);

		log_write("metacall", LOG_LEVEL_DEBUG, "%s -> %d | %d", key_array[i], *value0, *value1);

		vector_destroy(v);
	}

	EXPECT_EQ((size_t)key_array_size * 2, (size_t)map_size(m));

	/* Remove value */
	for (size_t i = 0; i < key_array_size; ++i)
	{
		int *remove_value = (int *)map_remove(m, key_array[i]);

		EXPECT_EQ((int)(value_array[i] == *remove_value || value_array[value_array_size - 1 - i] == *remove_value), (int)1);
	}

	EXPECT_EQ((size_t)key_array_size, (size_t)map_size(m));

	/* Remove multiple values */
	EXPECT_EQ((int)0, (int)map_insert(m, key_array[0], &value_array[5]));
	EXPECT_EQ((int)0, (int)map_insert(m, key_array[0], &value_array[6]));
	EXPECT_EQ((int)0, (int)map_insert(m, key_array[0], &value_array[7]));
	EXPECT_EQ((int)0, (int)map_insert(m, key_array[0], &value_array[8]));

	vector v = map_remove_all(m, key_array[0]);

	EXPECT_NE((vector)NULL, (vector)v);

	EXPECT_EQ((size_t)5, (size_t)vector_size(v));

	log_write("metacall", LOG_LEVEL_DEBUG, "Deleted %d", *(vector_at_type(v, 0, int *)));
	log_write("metacall", LOG_LEVEL_DEBUG, "Deleted %d", *(vector_at_type(v, 1, int *)));
	log_write("metacall", LOG_LEVEL_DEBUG, "Deleted %d", *(vector_at_type(v, 2, int *)));
	log_write("metacall", LOG_LEVEL_DEBUG, "Deleted %d", *(vector_at_type(v, 3, int *)));
	log_write("metacall", LOG_LEVEL_DEBUG, "Deleted %d", *(vector_at_type(v, 4, int *)));

	vector_destroy(v);

	log_write("metacall", LOG_LEVEL_DEBUG, "vvvvvvvvvvvvvvvv - This invalid message is correct, we are testing for a invalid remove");
	v = map_remove_all(m, key_array[0]);
	log_write("metacall", LOG_LEVEL_DEBUG, "^^^^^^^^^^^^^^^^ - This invalid message is correct, we are testing for a invalid remove");

	EXPECT_EQ((vector)NULL, (vector)v);

	EXPECT_EQ((size_t)key_array_size - 1, (size_t)map_size(m));

	map_destroy(m);
}

TEST_F(adt_map_test, map_structs)
{
	typedef struct stru_type
	{
		const char *str;
		int a;
	} * stru;

	static const char k[] = "random";

	stru a = new stru_type();
	a->str = k;
	a->a = 123;

	stru b = new stru_type();
	b->str = k;
	b->a = 321;

	map m = map_create(&hash_callback_str, &comparable_callback_str);

	EXPECT_EQ((int)0, (int)map_insert(m, (map_key)a->str, a));
	EXPECT_EQ((int)0, (int)map_insert(m, (map_key)b->str, b));

	vector v = map_get(m, (map_key)k);

	EXPECT_EQ((size_t)vector_size(v), (size_t)2);

	stru value0 = vector_at_type(v, 0, stru);

	EXPECT_EQ((int)(123 == value0->a || 321 == value0->a), (int)1);

	stru value1 = vector_at_type(v, 1, stru);

	EXPECT_EQ((int)(123 == value1->a || 321 == value1->a), (int)1);

	EXPECT_EQ((int)(a == value0 || a == value1), (int)1);
	EXPECT_EQ((int)(b == value0 || b == value1), (int)1);

	log_write("metacall", LOG_LEVEL_DEBUG, "A-B %p | %p", a, b);
	log_write("metacall", LOG_LEVEL_DEBUG, "val %p | %p", value0, value1);

	vector_destroy(v);

	map_destroy(m);

	delete a;
	delete b;
}
