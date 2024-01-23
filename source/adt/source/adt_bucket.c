/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

/* -- Headers -- */

#include <adt/adt_bucket.h>

#include <log/log.h>

#include <string.h>

/* -- Definitions -- */

#define BUCKET_PAIRS_DEFAULT ((size_t)0x04)

/* -- Methods -- */

size_t bucket_capacity(size_t prime)
{
	static const size_t capacity_primes[] = {
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

bucket bucket_create(size_t size)
{
	bucket buckets;

	size_t iterator;

	if (size == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid bucket size: %" PRIuS, size);
		return NULL;
	}

	buckets = malloc(sizeof(struct bucket_type) * size);

	if (buckets == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Bad allocation for bucket");
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

int bucket_alloc_pairs(bucket b, size_t capacity)
{
	if (b == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid bucket");
		return 1;
	}

	if (b->pairs != NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Set bucket pairs already allocated");
		return 1;
	}

	b->pairs = malloc(sizeof(struct pair_type) * capacity);

	if (b->pairs == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Bad pairs allocation");
		return 1;
	}

	b->count = 0;
	b->capacity = capacity;

	return 0;
}

int bucket_realloc_pairs(bucket b, size_t new_capacity)
{
	pair pairs = realloc(b->pairs, sizeof(struct pair_type) * new_capacity);

	if (pairs == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Bad pairs reallocation");
		return 1;
	}

	b->pairs = pairs;
	b->capacity = new_capacity;

	return 0;
}

pair bucket_get_pair(bucket b, comparable_callback compare_cb, void *key)
{
	size_t iterator;

	if (b->pairs == NULL || b->count == 0)
	{
		return NULL;
	}

	for (iterator = 0; iterator < b->count; ++iterator)
	{
		pair p = &b->pairs[iterator];

		if (compare_cb(key, p->key) == 0)
		{
			return p;
		}
	}

	return NULL;
}

vector bucket_get_pairs_value(bucket b, comparable_callback compare_cb, void *key)
{
	size_t iterator;
	vector v;

	if (b->pairs == NULL)
	{
		return NULL;
	}

	v = vector_create(sizeof(void *));

	for (iterator = 0; iterator < b->count; ++iterator)
	{
		pair p = &b->pairs[iterator];

		if (compare_cb(key, p->key) == 0)
		{
			vector_push_back(v, &p->value);
		}
	}

	return v;
}

int bucket_insert(bucket b, void *key, void *value)
{
	pair p;

	if (b == NULL || key == NULL || value == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid bucket insert parameters");
		return 1;
	}

	if (b->pairs == NULL && bucket_alloc_pairs(b, BUCKET_PAIRS_DEFAULT) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid bucket insertion pairs allocation");
		return 1;
	}

	if ((b->count + 1) >= b->capacity)
	{
		if (bucket_realloc_pairs(b, b->capacity << 1) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid bucket insertion pairs reallocation");
			return 1;
		}
	}

	p = &b->pairs[b->count];

	p->key = key;
	p->value = value;

	++b->count;

	return 0;
}

int bucket_remove(bucket b, comparable_callback compare_cb, void *key, void **value)
{
	size_t iterator;

	if (b == NULL || compare_cb == NULL || key == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid bucket remove parameters");
		return 1;
	}

	if (b->pairs == NULL || b->count == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid bucket remove pairs");
		return 1;
	}

	for (iterator = 0; iterator < b->count; ++iterator)
	{
		pair p = &b->pairs[iterator];

		if (compare_cb(key, p->key) == 0)
		{
			void *deleted_value = p->value;

			size_t next = iterator + 1;

			size_t new_capacity = b->capacity >> 1;

			memmove(p, &b->pairs[next], sizeof(struct pair_type) * (b->count - next));

			--b->count;

			if (b->count <= new_capacity && new_capacity > BUCKET_PAIRS_DEFAULT)
			{
				if (bucket_realloc_pairs(b, new_capacity) != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Invalid bucket remove pairs reallocation");
					return 1;
				}
			}

			if (value != NULL)
			{
				*value = deleted_value;
			}

			return 0;
		}
	}

	return 1;
}
