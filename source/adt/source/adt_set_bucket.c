/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

/* -- Headers -- */

#include <adt/adt_set_bucket.h>

#include <log/log.h>

#include <string.h>

/* -- Definitions -- */

#define SET_BUCKET_PAIRS_DEFAULT	((size_t)0x04)

/* -- Methods -- */

size_t set_bucket_capacity(size_t prime)
{
	static const size_t capacity_primes[] =
	{
		5UL,
		11UL,
		23UL,
		47UL,
		97UL,
		199UL,
		409UL,
		823UL,
		1741UL,
		3469UL,
		6949UL,
		14033UL,
		28411UL,
		57557UL,
		116731UL,
		236897UL,
		480881UL,
		976369UL,
		1982627UL,
		4026031UL,
		8175383UL,
		16601593UL,
		33712729UL,
		68460391UL,
		139022417UL,
		282312799UL,
		573292817UL,
		1164186217UL,
		2364114217UL,
		4294967291UL,

		#if (SIZE_MAX > 0xFFFFFFFF)
			(size_t)8589934583ULL,
			(size_t)17179869143ULL,
			(size_t)34359738337ULL,
			(size_t)68719476731ULL,
			(size_t)137438953447ULL,
			(size_t)274877906899ULL,
			(size_t)549755813881ULL,
			(size_t)1099511627689ULL,
			(size_t)2199023255531ULL,
			(size_t)4398046511093ULL,
			(size_t)8796093022151ULL,
			(size_t)17592186044399ULL,
			(size_t)35184372088777ULL,
			(size_t)70368744177643ULL,
			(size_t)140737488355213ULL,
			(size_t)281474976710597ULL,
			(size_t)562949953421231ULL,
			(size_t)1125899906842597ULL,
			(size_t)2251799813685119ULL,
			(size_t)4503599627370449ULL,
			(size_t)9007199254740881ULL,
			(size_t)18014398509481951ULL,
			(size_t)36028797018963913ULL,
			(size_t)72057594037927931ULL,
			(size_t)144115188075855859ULL,
			(size_t)288230376151711717ULL,
			(size_t)576460752303423433ULL,
			(size_t)1152921504606846883ULL,
			(size_t)2305843009213693951ULL,
			(size_t)4611686018427387847ULL,
			(size_t)9223372036854775783ULL,
			(size_t)18446744073709551557ULL
		#endif
	};

	static const size_t capacity_primes_size = sizeof(capacity_primes) / sizeof(capacity_primes[0]);

	if (prime < capacity_primes_size)
	{
		return capacity_primes[prime];
	}

	return 0;
}

set_bucket set_bucket_create(size_t size)
{
	set_bucket buckets;

	size_t iterator;

	if (size == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set bucket size: %" PRIuS, size);

		return NULL;
	}

	buckets = malloc(sizeof(struct set_bucket_type) * size);

	if (buckets == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Bad allocation for set bucket");

		return NULL;
	}

	for (iterator = 0; iterator < size; ++iterator)
	{
		buckets[iterator].count = 0;
		buckets[iterator].capacity = 0;
		buckets[iterator].pairs = NULL;
	}

	return buckets;
}

int set_bucket_alloc_pairs(set_bucket bucket, size_t capacity)
{
	if (bucket == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set bucket");

		return 1;
	}

	if (bucket->pairs != NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Set bucket pairs already allocated");

		return 1;
	}

	bucket->pairs = malloc(sizeof(struct set_pair_type) * capacity);

	if (bucket->pairs == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Bad set pairs allocation");

		return 1;
	}

	bucket->count = 0;
	bucket->capacity = capacity;

	return 0;
}

int set_bucket_realloc_pairs(set_bucket bucket, size_t new_capacity)
{
	set_pair pairs = realloc(bucket->pairs, sizeof(struct set_pair_type) * new_capacity);

	if (pairs == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Bad set pairs reallocation");

		return 1;
	}

	bucket->pairs = pairs;
	bucket->capacity = new_capacity;

	return 0;
}

set_pair set_bucket_get_pair(set_bucket bucket, comparable_callback compare_cb, void * key)
{
	size_t iterator;

	if (bucket->pairs == NULL || bucket->count == 0)
	{
		return NULL;
	}

	for (iterator = 0; iterator < bucket->count; ++iterator)
	{
		set_pair pair = &bucket->pairs[iterator];

		if (compare_cb(key, pair->key) == 0)
		{
			return pair;
		}
	}

	return NULL;
}

int set_bucket_insert(set_bucket bucket, void * key, void * value)
{
	set_pair pair;

	if (bucket == NULL || key == NULL || value == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set bucket insert parameters");

		return 1;
	}

	if (bucket->pairs == NULL && set_bucket_alloc_pairs(bucket, SET_BUCKET_PAIRS_DEFAULT) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set bucket insertion pairs allocation");

		return 1;
	}

	if ((bucket->count + 1) >= bucket->capacity)
	{
		if (set_bucket_realloc_pairs(bucket, bucket->capacity << 1) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid set bucket insertion pairs reallocation");
	
			return 1;
		}
	}

	pair = &bucket->pairs[bucket->count];

	pair->key = key;
	pair->value = value;

	++bucket->count;

	return 0;
}

int set_bucket_remove(set_bucket bucket, comparable_callback compare_cb, void * key, void ** value)
{
	size_t iterator;

	if (bucket == NULL || compare_cb == NULL || key == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set bucket remove parameters");

		return 1;
	}

	if (bucket->pairs == NULL || bucket->count == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set bucket remove pairs");

		return 1;
	}

	for (iterator = 0; iterator < bucket->count; ++iterator)
	{
		set_pair pair = &bucket->pairs[iterator];

		if (compare_cb(key, pair->key) == 0)
		{
			void * deleted_value = pair->value;

			size_t next = iterator + 1;

			size_t new_capacity = bucket->capacity >> 1;

			memmove(pair, &bucket->pairs[next], sizeof(struct set_pair_type) * (bucket->count - next));

			--bucket->count;

			if (bucket->count <= new_capacity && new_capacity > SET_BUCKET_PAIRS_DEFAULT)
			{	
				if (set_bucket_realloc_pairs(bucket, new_capacity) != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Invalid set bucket remove pairs reallocation");
			
					return 1;
				}
			}

			*value = deleted_value;

			return 0;
		}
	}

	return 1;
}
