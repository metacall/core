/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

#include <gmock/gmock.h>

#include <cstdio>
#include <cstring>

#include <adt/vector.h>
#include <adt/trie.h>

class trie_test : public testing::Test
{
  public:
};

int trie_iterator_cb_print(trie t, trie_key key, trie_value value, trie_cb_iterate_args args)
{
	(void)args;

	if (t != NULL && key != NULL && value != NULL)
	{
		const char * key_str = reinterpret_cast<const char *>(key);
		const char * value_str = reinterpret_cast<const char *>(value);

		printf("%s : %s\n", key_str, value_str);

		return 0;
	}

	return 1;
}

int trie_iterator_cb_clear(trie t, trie_key key, trie_value value, trie_cb_iterate_args args)
{
	(void)args;

	if (t != NULL && key != NULL && value != NULL)
	{
		free(key);

		free(value);

		return 0;
	}

	return 1;
}

TEST_F(trie_test, DefaultConstructor)
{
	size_t iterator;

	static const char * keys_str[] =
	{
		"this", "is", "a", "path"
	};

	static const char * values_str[] =
	{
		"these", "are", "the", "values"
	};

	trie t = trie_create(&hash_callback_str, &comparable_callback_str);

	vector keys = vector_create(sizeof(trie_key));

	vector keys_copy = NULL;

	vector prefixes = vector_create(sizeof(trie_key));

	trie suffix_trie = NULL;

	trie_key * last_prefix;

	size_t keys_size = sizeof(keys_str) / sizeof(keys_str[0]);

	for (iterator = 0; iterator < keys_size; ++iterator)
	{
		size_t key_size = strlen(keys_str[iterator]) + 1;
		size_t value_size = strlen(values_str[iterator]) + 1;

		trie_key key = malloc(key_size);
		trie_value value = malloc(value_size);

		memcpy(key, keys_str[iterator], key_size);
		memcpy(value, values_str[iterator], value_size);

		vector_push_back(keys, &key);

		EXPECT_EQ((int) 0, (int) trie_insert(t, keys, value));
	}

	keys_copy = vector_copy(keys);

	for (iterator = 0; iterator < keys_size; ++iterator)
	{
		trie_value value = trie_get(t, keys_copy);

		const char * value_str = reinterpret_cast<const char *>(value);

		printf("%lu -> %s\n", iterator, value_str);

		EXPECT_EQ((int) 0, (int) strcmp(values_str[keys_size - iterator - 1], value_str));

		vector_pop_back(keys_copy);
	}

	vector_destroy(keys_copy);

	last_prefix = reinterpret_cast<trie_key *>(vector_back(keys));

	EXPECT_EQ((int) 0, (int) trie_prefixes(t, *last_prefix, prefixes));

	printf("cannonical path: ");

	for (iterator = 0; iterator < vector_size(prefixes); ++iterator)
	{
		trie_key * key = reinterpret_cast<trie_key *>(vector_at(prefixes, iterator));

		const char * key_str = reinterpret_cast<const char *>(*key);

		printf("%s/", key_str);

		EXPECT_EQ((int) 0, (int) strcmp(keys_str[iterator], key_str));
	}

	printf("\n");

	vector_pop_back(keys);

	last_prefix = reinterpret_cast<trie_key *>(vector_back(keys));

	suffix_trie = trie_suffixes(t, *last_prefix);

	trie_iterate(t, &trie_iterator_cb_print, NULL);

	trie_iterate(suffix_trie, &trie_iterator_cb_print, NULL);

	trie_iterate(t, &trie_iterator_cb_clear, NULL);

	vector_destroy(prefixes);

	vector_destroy(keys);

	trie_destroy(suffix_trie);

	trie_destroy(t);
}
