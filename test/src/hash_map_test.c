#include <stdio.h>
#include <string.h>

#include <adt/hash_map.h>

static int iterator_counter = 0;

hash_map_hash hash_map_cb_hash_str(hash_map_key key)
{
	char * str = (char *)key;

	hash_map_hash hash = 0x1505;

	while (*str++ != '\0')
	{
		hash = (hash_map_hash)(((hash << 5) + hash) + *str);
	}

	return hash;
}

int hash_map_cb_compare_str(hash_map_key key_a, hash_map_key key_b)
{
	return strcmp((char *)key_a, (char *)key_b);
}

int hash_map_cb_iterate_str_to_int(hash_map map, hash_map_key key, hash_map_value value, hash_map_cb_iterate_args args)
{
	printf("%s -> %d\n", (char *)key, *((int *)(value)));

	++iterator_counter;

	return 0;
}

int main(int argc, char * argv[])
{
	hash_map map = hash_map_create(&hash_map_cb_hash_str, &hash_map_cb_compare_str);

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

	hash_map_insert(map, "aaaaaa", &value_array[0]);
	hash_map_insert(map, "aaaaab", &value_array[1]);
	hash_map_insert(map, "aaaaba", &value_array[2]);
	hash_map_insert(map, "aaaabb", &value_array[3]);
	hash_map_insert(map, "aaabaa", &value_array[4]);
	hash_map_insert(map, "aaabab", &value_array[5]);
	hash_map_insert(map, "aaabba", &value_array[6]);
	hash_map_insert(map, "aaabbb", &value_array[7]);
	hash_map_insert(map, "aabaaa", &value_array[8]);
	hash_map_insert(map, "aabaab", &value_array[9]);

	hash_map_insert(map, "aababa", &value_array[10]);
	hash_map_insert(map, "aababb", &value_array[11]);

printf("HELLO\n");

	hash_map_insert(map, "aabbaa", &value_array[12]);

printf("HELLO\n");

	hash_map_insert(map, "aabbab", &value_array[13]);
	hash_map_insert(map, "aabbba", &value_array[14]);
	hash_map_insert(map, "aabbbb", &value_array[15]);
	hash_map_insert(map, "abaaaa", &value_array[16]);
	hash_map_insert(map, "abaaab", &value_array[17]);
	hash_map_insert(map, "abaaba", &value_array[18]);
	hash_map_insert(map, "abaabb", &value_array[19]);

	hash_map_insert(map, "ababaa", &value_array[20]);
	hash_map_insert(map, "ababab", &value_array[21]);
	hash_map_insert(map, "ababbb", &value_array[22]);
	hash_map_insert(map, "abbaaa", &value_array[23]);
	hash_map_insert(map, "abbaab", &value_array[24]);
	hash_map_insert(map, "abbaba", &value_array[25]);
	hash_map_insert(map, "abbabb", &value_array[26]);
	hash_map_insert(map, "abbbaa", &value_array[27]);
	hash_map_insert(map, "abbbab", &value_array[28]);
	hash_map_insert(map, "abbbba", &value_array[29]);

	hash_map_insert(map, "abbbbb", &value_array[30]);
	hash_map_insert(map, "baaaaa", &value_array[31]);
	hash_map_insert(map, "baaaab", &value_array[32]);
	hash_map_insert(map, "baaaba", &value_array[33]);
	hash_map_insert(map, "baaabb", &value_array[34]);
	hash_map_insert(map, "baabaa", &value_array[35]);
	hash_map_insert(map, "baabab", &value_array[36]);
	hash_map_insert(map, "baabba", &value_array[37]);
	hash_map_insert(map, "baabbb", &value_array[38]);
	hash_map_insert(map, "babaaa", &value_array[39]);

	hash_map_insert(map, "babaab", &value_array[40]);
	hash_map_insert(map, "bababa", &value_array[41]);
	hash_map_insert(map, "bababb", &value_array[42]);
	hash_map_insert(map, "babbaa", &value_array[43]);
	hash_map_insert(map, "babbab", &value_array[44]);
	hash_map_insert(map, "babbba", &value_array[45]);
	hash_map_insert(map, "babbbb", &value_array[46]);
	hash_map_insert(map, "bbaaaa", &value_array[47]);
	hash_map_insert(map, "bbaaab", &value_array[48]);
	hash_map_insert(map, "bbaaba", &value_array[49]);

	hash_map_insert(map, "bbaabb", &value_array[50]);
	hash_map_insert(map, "bbabaa", &value_array[51]);
	hash_map_insert(map, "bbabab", &value_array[52]);
	hash_map_insert(map, "bbabba", &value_array[53]);
	hash_map_insert(map, "bbabbb", &value_array[54]);
	hash_map_insert(map, "bbbaaa", &value_array[55]);
	hash_map_insert(map, "bbbaab", &value_array[56]);
	hash_map_insert(map, "bbbaba", &value_array[57]);
	hash_map_insert(map, "bbbabb", &value_array[58]);
	hash_map_insert(map, "bbbbaa", &value_array[59]);

	hash_map_insert(map, "bbbbab", &value_array[60]);
	hash_map_insert(map, "bbbbba", &value_array[61]);
	hash_map_insert(map, "bbbbbb", &value_array[62]);

	hash_map_iterate(map, &hash_map_cb_iterate_str_to_int, NULL);

	printf("total number of insertions: %d == %zu\n", iterator_counter, sizeof(value_array) / sizeof(value_array[0]));

	printf("\"babbaa\": %d == %d\n", *((int *)hash_map_get(map, "babbaa")), value_array[43]);

	hash_map_destroy(map);

	printf("Helloo\n");
	return 0;
}
