/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

#include <adt/adt_hash_map.h>

#include <log/log.h>

#define HASH_MAP_BUCKET_PAIRS_DEFAULT	0x04
#define HASH_MAP_BUCKET_PAIRS_LIMIT	0x40
#define HASH_MAP_BUCKET_RATIO_MIN	0.1f
#define HASH_MAP_BUCKET_RATIO_MAX	0.77f

typedef struct hash_map_pair_type
{
	hash_map_key key;
	hash_map_value value;

} * hash_map_pair;

typedef struct hash_map_bucket_type
{
	int count;
	int capacity;
	hash_map_pair pairs;

} * hash_map_bucket;

typedef struct hash_map_type
{
	int count;
	int capacity;
	size_t prime;
	hash_map_bucket buckets;
	hash_map_cb_hash hash_cb;
	hash_map_cb_compare compare_cb;
	int reallocating;
	int amount;

} * hash_map;

static int hash_map_bucket_capacity(size_t prime);

static int hash_map_bucket_create(hash_map map, size_t prime);

static int hash_map_bucket_alloc_pairs(hash_map_bucket bucket);

static int hash_map_bucket_realloc_pairs(hash_map_bucket bucket, int count);

static int hash_map_bucket_realloc_iterator(hash_map map, hash_map_key key, hash_map_value value, hash_map_cb_iterate_args args);

static int hash_map_bucket_realloc(hash_map map);

static hash_map_pair hash_map_bucket_get_pair(hash_map map, hash_map_bucket bucket, hash_map_key key);

static int hash_map_bucket_insert(hash_map map, hash_map_bucket bucket, hash_map_key key, hash_map_value value);

static int hash_map_append_cb_iterate(hash_map map, hash_map_key key, hash_map_value value, hash_map_cb_iterate_args args);

static int hash_map_bucket_capacity(size_t prime)
{
	static int capacity_primes[] =
	{
		/* todo: make a better (or configurable) policy */

		13, 31, 61, 127, 251, 509, 1021, 2039, 4093, 8191, 16381, 32749, 65521
	};

	if (prime < sizeof(capacity_primes) / sizeof(capacity_primes[0]))
	{
		return capacity_primes[prime];
	}

	log_write("metacall", LOG_LEVEL_ERROR, "Invalid hash map bucket capacity");

	return -1;
}

static int hash_map_bucket_create(hash_map map, size_t prime)
{
	int capacity = hash_map_bucket_capacity(prime);

	hash_map_bucket buckets = malloc(sizeof(struct hash_map_bucket_type) * capacity);

	if (buckets != NULL)
	{
		int i;

		for (i = 0; i < capacity; ++i)
		{
			buckets[i].count = 0;
			buckets[i].capacity = 0;
			buckets[i].pairs = NULL;
		}

		map->buckets = buckets;
		map->capacity = capacity;
		map->prime = prime;
		map->count = 0;

		return 0;
	}

	log_write("metacall", LOG_LEVEL_ERROR, "Bad allocation for hash map bucket");

	return 1;
}

hash_map hash_map_create(hash_map_cb_hash hash_cb, hash_map_cb_compare compare_cb)
{
	if (hash_cb != NULL && compare_cb != NULL)
	{
		hash_map map = malloc(sizeof(struct hash_map_type));

		if (map != NULL)
		{
			map->hash_cb = hash_cb;
			map->compare_cb = compare_cb;
			map->reallocating = 0;
			map->amount = 0;

			if (hash_map_bucket_create(map, 0) == 0)
			{
				return map;
			}

			free(map);
		}
	}

	log_write("metacall", LOG_LEVEL_ERROR, "Bad allocation for hash map");

	return NULL;
}

size_t hash_map_size(hash_map map)
{
	if (map != NULL)
	{
		return map->amount;
	}

	return 0;
}

static int hash_map_bucket_alloc_pairs(hash_map_bucket bucket)
{
	if (bucket)
	{
		if (bucket->pairs == NULL && bucket->capacity == 0)
		{
			bucket->pairs = malloc(sizeof(struct hash_map_pair_type) * HASH_MAP_BUCKET_PAIRS_DEFAULT);

			if (bucket->pairs)
			{
				bucket->count = 0;
				bucket->capacity = HASH_MAP_BUCKET_PAIRS_DEFAULT;

				return 0;
			}
			
			log_write("metacall", LOG_LEVEL_ERROR, "Bad allocation for hash map pairs");

			return 1;
		}

		return 0;
	}

	return 1;
}

static int hash_map_bucket_realloc_pairs(hash_map_bucket bucket, int count)
{
	if (hash_map_bucket_alloc_pairs(bucket) != 0)
	{
		return 1;
	}

	if (count >= bucket->capacity)
	{
		int new_capacity = bucket->capacity << 1;

		if (new_capacity <= HASH_MAP_BUCKET_PAIRS_LIMIT)
		{
			hash_map_pair pairs = realloc(bucket->pairs, sizeof(struct hash_map_pair_type) * new_capacity);

			if (pairs != NULL)
			{
				bucket->pairs = pairs;
				bucket->capacity = new_capacity;

				return 0;
			}
		}

		log_write("metacall", LOG_LEVEL_ERROR, "Bad reallocation for hash map pairs");

		return 1;
	}

	return 0;
}

static int hash_map_bucket_realloc_iterator(hash_map map, hash_map_key key, hash_map_value value, hash_map_cb_iterate_args args)
{
	hash_map new_map = (hash_map)args;

	if (new_map != map && key != NULL && value != NULL)
	{
		return hash_map_insert(new_map, key, value);
	}

	return 1;
}

static int hash_map_bucket_realloc(hash_map map)
{
	struct hash_map_type new_map;

	size_t prime = map->prime;

	float ratio = (float)((float)map->count / (float)map->capacity);

	if (map->reallocating == 0 && (prime > 0 && ratio <= HASH_MAP_BUCKET_RATIO_MIN))
	{
		--prime;
	}
	else if (ratio >= HASH_MAP_BUCKET_RATIO_MAX)
	{
		++prime;
	}
	else
	{
		return 0;
	}

	new_map.hash_cb = map->hash_cb;
	new_map.compare_cb = map->compare_cb;
	new_map.reallocating = 1;

	if (hash_map_bucket_create(&new_map, prime) == 0)
	{
		hash_map_iterate(map, &hash_map_bucket_realloc_iterator, &new_map);

		map->count = new_map.count;
		map->capacity = new_map.capacity;
		map->prime = new_map.prime;

		map->reallocating = 0;

		free(map->buckets);

		map->buckets = new_map.buckets;

		return 0;
	}

	return 1;
}

static hash_map_pair hash_map_bucket_get_pair(hash_map map, hash_map_bucket bucket, hash_map_key key)
{
	if (bucket->pairs != NULL && bucket->count > 0)
	{
		int i;

		for (i = 0; i < bucket->count; ++i)
		{
			hash_map_pair pair = &bucket->pairs[i];

			if (map->compare_cb(key, pair->key) == 0)
			{
				return pair;
			}
		}
	}

	return NULL;
}

static int hash_map_bucket_insert(hash_map map, hash_map_bucket bucket, hash_map_key key, hash_map_value value)
{
	if (bucket->pairs != NULL)
	{
		hash_map_pair pair;

		int pair_count = bucket->count + 1;

		if (hash_map_bucket_realloc_pairs(bucket, pair_count) != 0)
		{
			int map_count = map->count;

			if (hash_map_bucket_realloc(map) == 0)
			{
				if (map_count != map->count)
				{
					return hash_map_insert(map, key, value);
				}
			}

			return 1;
		}

		pair = &bucket->pairs[bucket->count];

		pair->key = key;
		pair->value = value;

		bucket->count = pair_count;

		return 0;
	}

	bucket->capacity = 0;

	return 1;
}

int hash_map_insert(hash_map map, hash_map_key key, hash_map_value value)
{
	if (map != NULL && key != NULL && value != NULL)
	{
		hash_map_hash h = map->hash_cb(key);

		int index = h % map->capacity;

		hash_map_bucket bucket = &map->buckets[index];

		if (bucket->pairs == NULL)
		{
			++map->count;
		}

		if (hash_map_bucket_realloc(map) == 0)
		{
			index = h % map->capacity;

			bucket = &map->buckets[index];

			if (hash_map_bucket_alloc_pairs(bucket) == 0)
			{
				if (hash_map_bucket_insert(map, bucket, key, value) == 0)
				{
					++map->amount;

					return 0;
				}

				log_write("metacall", LOG_LEVEL_ERROR, "Invalid hash map bucket insertion");
			}

			log_write("metacall", LOG_LEVEL_ERROR, "Invalid hash map bucket pairs allocation");
		}

		log_write("metacall", LOG_LEVEL_ERROR, "Invalid hash map bucket pairs reallocation");

		--map->count;
	}

	return 1;
}

hash_map_value hash_map_get(hash_map map, const hash_map_key key)
{
	if (map != NULL && key != NULL)
	{
		hash_map_hash h = map->hash_cb(key);

		int index = h % map->capacity;

		hash_map_bucket bucket = &map->buckets[index];

		hash_map_pair pair = hash_map_bucket_get_pair(map, bucket, key);

		if (pair != NULL)
		{
			return pair->value;
		}
	}

	return NULL;
}

hash_map_value hash_map_remove(hash_map map, hash_map_key key)
{
	if (map != NULL && key != NULL)
	{
		hash_map_hash h = map->hash_cb(key);

		int index = h % map->capacity;

		hash_map_bucket bucket = &map->buckets[index];

		if (bucket->pairs != NULL && bucket->count > 0)
		{
			hash_map_value value = NULL;

			int i;

			for (i = 0; i < bucket->count; ++i)
			{
				hash_map_pair pair = &bucket->pairs[i];

				if (map->compare_cb(key, pair->key) == 0)
				{
					value = pair->value;

					for ( ; i < bucket->count - 1; ++i)
					{
						hash_map_pair next = &bucket->pairs[i + 1];

						pair = &bucket->pairs[i];

						pair->key = next->key;
						pair->value = next->value;
					}

					--bucket->count;
				}
			}

			if (bucket->count == 0)
			{
				--map->count;

				if (hash_map_bucket_realloc(map) != 0)
				{
					return NULL;
				}
			}

			if (value != NULL)
			{
				--map->amount;
			}

			return value;
		}
	}

	return NULL;
}

void hash_map_iterate(hash_map map, hash_map_cb_iterate iterate_cb, hash_map_cb_iterate_args args)
{
	if (map != NULL && map->buckets != NULL && iterate_cb != NULL)
	{
		int i;

		for (i = 0; i < map->capacity; ++i)
		{
			hash_map_bucket bucket = &map->buckets[i];

			if (bucket->pairs != NULL && bucket->count > 0)
			{
				int j;

				for (j = 0; j < bucket->count; ++j)
				{
					hash_map_pair pair = &bucket->pairs[j];

					if (iterate_cb(map, pair->key, pair->value, args) != 0)
					{
						return;
					}
				}
			}
		}
	}
}

static int hash_map_append_cb_iterate(hash_map map, hash_map_key key, hash_map_value value, hash_map_cb_iterate_args args)
{
	hash_map dest = (hash_map)args;

	(void)map;

	return hash_map_insert(dest, key, value);
}

int hash_map_append(hash_map dest, hash_map src)
{
	hash_map_cb_iterate_args args = (hash_map_cb_iterate_args)dest;

	hash_map_iterate(src, &hash_map_append_cb_iterate, args);

	return 0;
}

int hash_map_clear(hash_map map)
{
	if (map != NULL)
	{
		if (map->buckets != NULL)
		{
			int i;

			for (i = 0; i < map->capacity; ++i)
			{
				hash_map_bucket bucket = &map->buckets[i];

				if (bucket->pairs != NULL)
				{
					free(bucket->pairs);
				}
			}

			free(map->buckets);
		}

		map->reallocating = 0;
		map->amount = 0;

		if (hash_map_bucket_create(map, 0) == 0)
		{
			return 0;
		}
	}

	return 1;
}

void hash_map_destroy(hash_map map)
{
	if (map != NULL)
	{
		if (map->buckets != NULL)
		{
			int i;

			for (i = 0; i < map->capacity; ++i)
			{
				hash_map_bucket bucket = &map->buckets[i];

				if (bucket->pairs != NULL)
				{
					free(bucket->pairs);
				}
			}

			free(map->buckets);
		}

		free(map);
	}
}
