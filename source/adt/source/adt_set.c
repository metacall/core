/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

/* -- Headers -- */

#include <adt/adt_set.h>

#include <log/log.h>

/* -- Definitions -- */

#define SET_BUCKET_PAIRS_DEFAULT	0x04
#define SET_BUCKET_PAIRS_LIMIT		0x40
#define SET_BUCKET_RATIO_MIN		0.1f
#define SET_BUCKET_RATIO_MAX		0.77f

/* -- Member Data -- */

typedef struct set_pair_type
{
	set_key key;
	set_value value;

} * set_pair;

typedef struct set_bucket_type
{
	size_t count;
	size_t capacity;
	set_pair pairs;

} * set_bucket;

struct set_type
{
	size_t count;
	size_t capacity;
	size_t prime;
	set_bucket buckets;
	set_cb_hash hash_cb;
	set_cb_compare compare_cb;
	int reallocating;
	size_t amount;

};

struct set_iterator_type
{
	set s;
	size_t bucket;
	size_t pair;

};

/* -- Methods -- */

size_t set_bucket_capacity(size_t prime)
{
	static size_t capacity_primes[] =
	{
		/* todo: make a better (or configurable) policy */

		13, 31, 61, 127, 251, 509, 1021, 2039, 4093, 8191, 16381, 32749, 65521
	};

	if (prime < sizeof(capacity_primes) / sizeof(capacity_primes[0]))
	{
		return capacity_primes[prime];
	}

	log_write("metacall", LOG_LEVEL_ERROR, "Invalid set bucket capacity");

	return 0;
}

int set_bucket_create(set s, size_t prime)
{
	size_t capacity = set_bucket_capacity(prime);

	set_bucket buckets = malloc(sizeof(struct set_bucket_type) * capacity);

	size_t iterator;

	if (buckets == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Bad allocation for set bucket");

		return 1;
	}


	for (iterator = 0; iterator < capacity; ++iterator)
	{
		buckets[iterator].count = 0;
		buckets[iterator].capacity = 0;
		buckets[iterator].pairs = NULL;
	}

	s->buckets = buckets;
	s->capacity = capacity;
	s->prime = prime;
	s->count = 0;

	return 0;
}

set set_create(set_cb_hash hash_cb, set_cb_compare compare_cb)
{
	set s = NULL;

	if (hash_cb == NULL || compare_cb == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set creation parameters");

		return NULL;
	}

	s = malloc(sizeof(struct set_type));

	if (s == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Bad set allocation");
	}

	s->hash_cb = hash_cb;
	s->compare_cb = compare_cb;
	s->reallocating = 0;
	s->amount = 0;

	if (set_bucket_create(s, 0) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Bad set bucket creation");

		free(s);

		return NULL;
	}

	return s;
}

size_t set_size(set s)
{
	if (s != NULL)
	{
		return s->amount;
	}

	return 0;
}

int set_bucket_alloc_pairs(set_bucket bucket)
{
	if (bucket == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid null bucket");

		return 1;
	}

	if (bucket->pairs == NULL && bucket->capacity == 0)
	{
		bucket->pairs = malloc(sizeof(struct set_pair_type) * SET_BUCKET_PAIRS_DEFAULT);

		if (bucket->pairs == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Bad set pairs allocation");

			return 1;
		}

		bucket->count = 0;
		bucket->capacity = SET_BUCKET_PAIRS_DEFAULT;
	}

	return 0;
}

int set_bucket_realloc_pairs(set_bucket bucket, size_t count)
{
	if (set_bucket_alloc_pairs(bucket) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set pairs creation");

		return 1;
	}

	if (count >= bucket->capacity)
	{
		size_t new_capacity = bucket->capacity << 1;

		set_pair pairs = NULL;

		if (new_capacity > SET_BUCKET_PAIRS_LIMIT)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Set capacity overflow");

			return 1;
		}

		pairs = realloc(bucket->pairs, sizeof(struct set_pair_type) * new_capacity);

		if (pairs == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Bad set pairs reallocation");

			return 1;
		}

		bucket->pairs = pairs;
		bucket->capacity = new_capacity;
	}

	return 0;
}

int set_bucket_realloc_iterator(set s, set_key key, set_value value, set_cb_iterate_args args)
{
	set new_set = (set)args;

	(void)s;

	if (new_set != s && key != NULL && value != NULL)
	{
		return set_insert(new_set, key, value);
	}

	return 1;
}

int set_bucket_realloc(set s)
{
	struct set_type new_set;

	size_t prime = s->prime;

	float ratio = (float)((float)s->count / (float)s->capacity);

	if (s->reallocating == 0 && (prime > 0 && ratio <= SET_BUCKET_RATIO_MIN))
	{
		--prime;
	}
	else if (ratio >= SET_BUCKET_RATIO_MAX)
	{
		++prime;
	}
	else
	{
		return 0;
	}

	new_set.hash_cb = s->hash_cb;
	new_set.compare_cb = s->compare_cb;
	new_set.reallocating = 1;

	if (set_bucket_create(&new_set, prime) == 0)
	{
		set_iterate(s, &set_bucket_realloc_iterator, &new_set);

		s->count = new_set.count;
		s->capacity = new_set.capacity;
		s->prime = new_set.prime;

		s->reallocating = 0;

		free(s->buckets);

		s->buckets = new_set.buckets;

		return 0;
	}

	return 1;
}

set_pair set_bucket_get_pair(set s, set_bucket bucket, set_key key)
{
	if (bucket->pairs != NULL && bucket->count > 0)
	{
		size_t i;

		for (i = 0; i < bucket->count; ++i)
		{
			set_pair pair = &bucket->pairs[i];

			if (s->compare_cb(key, pair->key) == 0)
			{
				return pair;
			}
		}
	}

	return NULL;
}

int set_bucket_insert(set s, set_bucket bucket, set_key key, set_value value)
{
	if (bucket->pairs != NULL)
	{
		set_pair pair = set_bucket_get_pair(s, bucket, key);

		if (pair != NULL)
		{
			pair->value = value;

			return 0;
		}
		else
		{
			size_t pair_count = bucket->count + 1;

			if (set_bucket_realloc_pairs(bucket, pair_count) != 0)
			{
				size_t set_count = s->count;

				if (set_bucket_realloc(s) == 0)
				{
					if (set_count != s->count)
					{
						return set_insert(s, key, value);
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
	}

	bucket->capacity = 0;

	return 1;
}

int set_insert(set s, set_key key, set_value value)
{
	set_hash h;

	size_t index;

	set_bucket bucket;

	if (s == NULL || key == NULL || value == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set insertion parameters");

		return 1;
	}

	h = s->hash_cb(key);

	index = h % s->capacity;

	bucket = &s->buckets[index];

	if (bucket->pairs == NULL)
	{
		++s->count;
	}

	if (set_bucket_realloc(s) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set bucket reallocation");

		--s->count;

		return 1;
	}

	index = h % s->capacity;

	bucket = &s->buckets[index];

	if (set_bucket_alloc_pairs(bucket) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set bucket pairs allocation");

		--s->count;

		return 1;
	}

	if (set_bucket_insert(s, bucket, key, value) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set bucket insertion");

		--s->count;

		return 1;
	}

	++s->amount;

	return 0;
}

int set_insert_array(set s, set_key keys[], set_value values[], size_t size)
{
	size_t index;

	if (s == NULL || keys == NULL || values == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set insertion parameters");

		return 1;
	}

	for (index = 0; index < size; ++index)
	{
		if (set_insert(s, keys[index], values[index]) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid set array insertion");

			return 1;
		}
	}

	return 0;
}

set_value set_get(set s, set_key key)
{
	if (s != NULL && key != NULL)
	{
		set_hash hash = s->hash_cb(key);

		size_t index = hash % s->capacity;

		set_bucket bucket = &s->buckets[index];

		set_pair pair = set_bucket_get_pair(s, bucket, key);

		if (pair != NULL)
		{
			return pair->value;
		}
	}

	return NULL;
}

int set_contains(set s, set_key key)
{
	if (s != NULL && key != NULL)
	{
		set_hash hash = s->hash_cb(key);

		size_t index = hash % s->capacity;

		set_bucket bucket = &s->buckets[index];

		set_pair pair = set_bucket_get_pair(s, bucket, key);

		if (pair != NULL)
		{
			return 0;
		}
	}

	return 1;
}

set_value set_remove(set s, set_key key)
{
	if (s != NULL && key != NULL)
	{
		set_hash hash = s->hash_cb(key);

		size_t index = hash % s->capacity;

		set_bucket bucket = &s->buckets[index];

		if (bucket->pairs != NULL && bucket->count > 0)
		{
			set_value value = NULL;

			size_t i;

			for (i = 0; i < bucket->count; ++i)
			{
				set_pair pair = &bucket->pairs[i];

				if (s->compare_cb(key, pair->key) == 0)
				{
					value = pair->value;

					for ( ; i < bucket->count - 1; ++i)
					{
						set_pair next = &bucket->pairs[i + 1];

						pair = &bucket->pairs[i];

						pair->key = next->key;
						pair->value = next->value;
					}

					--bucket->count;
				}
			}

			if (bucket->count == 0)
			{
				--s->count;

				if (set_bucket_realloc(s) != 0)
				{
					return NULL;
				}
			}

			if (value != NULL)
			{
				--s->amount;
			}

			return value;
		}
	}

	return NULL;
}

void set_iterate(set s, set_cb_iterate iterate_cb, set_cb_iterate_args args)
{
	if (s != NULL && s->buckets != NULL && iterate_cb != NULL)
	{
		size_t i;

		for (i = 0; i < s->capacity; ++i)
		{
			set_bucket bucket = &s->buckets[i];

			if (bucket->pairs != NULL && bucket->count > 0)
			{
				size_t j;

				for (j = 0; j < bucket->count; ++j)
				{
					set_pair pair = &bucket->pairs[j];

					if (iterate_cb(s, pair->key, pair->value, args) != 0)
					{
						return;
					}
				}
			}
		}
	}
}

int set_append_cb_iterate(set s, set_key key, set_value value, set_cb_iterate_args args)
{
	set dest = (set)args;

	(void)s;

	return set_insert(dest, key, value);
}

int set_append(set dest, set src)
{
	set_cb_iterate_args args = (set_cb_iterate_args)dest;

	set_iterate(src, &set_append_cb_iterate, args);

	return 0;
}

int set_clear(set s)
{
	if (s != NULL)
	{
		if (s->buckets != NULL)
		{
			size_t i;

			for (i = 0; i < s->capacity; ++i)
			{
				set_bucket bucket = &s->buckets[i];

				if (bucket->pairs != NULL)
				{
					free(bucket->pairs);
				}
			}

			free(s->buckets);
		}

		s->reallocating = 0;
		s->amount = 0;

		if (set_bucket_create(s, 0) == 0)
		{
			return 0;
		}
	}

	return 1;
}

void set_destroy(set s)
{
	if (s != NULL)
	{
		if (s->buckets != NULL)
		{
			size_t i;

			for (i = 0; i < s->capacity; ++i)
			{
				set_bucket bucket = &s->buckets[i];

				if (bucket->pairs != NULL)
				{
					free(bucket->pairs);
				}
			}

			free(s->buckets);
		}

		free(s);
	}
}

set_iterator set_iterator_begin(set s)
{
	if (s != NULL && s->buckets != NULL && set_size(s) > 0)
	{
		set_iterator it = malloc(sizeof(struct set_iterator_type));

		if (it != NULL)
		{
			it->s = s;
			it->bucket = 0;
			it->pair = 0;

			set_iterator_next(it);

			return it;
		}
	}

	return NULL;
}

set_key set_iterator_get_key(set_iterator it)
{
	if (it != NULL && it->bucket < it->s->capacity && it->pair > 0)
	{
		return it->s->buckets[it->bucket].pairs[it->pair - 1].key;
	}

	return NULL;
}

set_value set_iterator_get_value(set_iterator it)
{
	if (it != NULL && it->bucket < it->s->capacity && it->pair > 0)
	{
		return it->s->buckets[it->bucket].pairs[it->pair - 1].value;
	}

	return NULL;
}

void set_iterator_next(set_iterator it)
{
	if (it != NULL)
	{
		for ( ; it->bucket < it->s->capacity; ++it->bucket)
		{
			set_bucket bucket = &it->s->buckets[it->bucket];

			if (bucket->pairs != NULL && bucket->count > 0)
			{
				for ( ; it->pair < bucket->count; ++it->pair)
				{
					set_pair pair = &bucket->pairs[it->pair];

					if (pair != NULL)
					{
						++it->pair;

						return;
					}
				}
			}
		}
	}
}

int set_iterator_end(set_iterator * it)
{
	if (it != NULL && *it != NULL)
	{
		if ((*it)->bucket >= (*it)->s->capacity)
		{
			free(*it);

			*it = NULL;

			return 0;
		}

		return 1;
	}

	return 0;
}
