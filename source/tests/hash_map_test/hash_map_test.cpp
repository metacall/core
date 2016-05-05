/************************************************************************/
/*	Abstract Data Type Library by Parra Studios							*/
/*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																		*/
/*	A abstract data type library providing generic containers.			*/
/*																		*/
/************************************************************************/

#include <gmock/gmock.h>

#include <cstdio>
#include <cstring>

#include <adt/hash_map.h>
#include <adt/hash_map_str.h>

typedef char key_str[7];

static size_t iterator_counter = 0;

int hash_map_cb_iterate_str_to_int(hash_map map, hash_map_key key, hash_map_value value, hash_map_cb_iterate_args args)
{
	if (map && args == NULL)
	{
		printf("%s -> %d\n", (char *)key, *((int *)(value)));

		++iterator_counter;

		return 0;
	}

	return 1;
}

class hash_map_test : public testing::Test
{
  public:
};

TEST_F(hash_map_test, CheckSomeResults)
{
	hash_map map = hash_map_create(&hash_map_cb_hash_str, &hash_map_cb_compare_str);

	static key_str key_array[] =
	{
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

	static int value_array[] =
	{
		10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
		20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
		30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
		40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
		50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
		60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
		70, 71, 72
	};

	EXPECT_EQ((size_t) sizeof(key_array) / sizeof(key_array[0]), (size_t) sizeof(value_array) / sizeof(value_array[0]));

	for (size_t i = 0; i < sizeof(key_array) / sizeof(key_array[0]); ++i)
	{
		EXPECT_EQ((int) hash_map_insert(map, key_array[i], &value_array[i]), (int) 0);
	}

	hash_map_iterate(map, &hash_map_cb_iterate_str_to_int, NULL);

	EXPECT_EQ((size_t) iterator_counter, (size_t) sizeof(value_array) / sizeof(value_array[0]));

	for (size_t i = 0; i < sizeof(key_array) / sizeof(key_array[0]); ++i)
	{
		int * value = (int *)hash_map_get(map, key_array[i]);

		EXPECT_NE((int *) value, (int *) NULL);

		EXPECT_EQ((int) *value, (int) value_array[i]);
	}

	hash_map_destroy(map);
}
