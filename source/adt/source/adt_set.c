/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
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

/* -- Headers -- */

#include <adt/adt_bucket.h>
#include <adt/adt_set.h>

#include <log/log.h>

/* -- Definitions -- */

#define SET_BUCKET_RATIO_MIN 0.1f
#define SET_BUCKET_RATIO_MAX 0.77f

/* -- Member Data -- */

struct set_type
{
	size_t count;
	size_t capacity;
	size_t prime;
	bucket buckets;
	set_cb_hash hash_cb;
	set_cb_compare compare_cb;
};

/* -- Methods -- */

set set_create(set_cb_hash hash_cb, set_cb_compare compare_cb)
{
	set s;

	if (hash_cb == NULL || compare_cb == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set creation parameters");
		return NULL;
	}

	s = malloc(sizeof(struct set_type));

	if (s == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Bad set allocation");
		return NULL;
	}

	s->hash_cb = hash_cb;
	s->compare_cb = compare_cb;
	s->count = 0;
	s->prime = 0;
	s->capacity = bucket_capacity(s->prime);
	s->buckets = bucket_create(s->capacity);

	if (s->buckets == NULL)
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
		return s->count;
	}

	return 0;
}

static int set_bucket_rehash(set s, set new_set)
{
	size_t bucket_iterator, pair_iterator;

	for (bucket_iterator = 0; bucket_iterator < s->capacity; ++bucket_iterator)
	{
		bucket b = &s->buckets[bucket_iterator];

		if (b->pairs != NULL && b->count > 0)
		{
			for (pair_iterator = 0; pair_iterator < b->count; ++pair_iterator)
			{
				pair p = &b->pairs[pair_iterator];

				set_hash h = new_set->hash_cb(p->key);

				size_t index = h % new_set->capacity;

				bucket b = &new_set->buckets[index];

				if (bucket_insert(b, p->key, p->value) != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Invalid set bucket realloc insertion");
					return 1;
				}

				++new_set->count;
			}
		}
	}

	return 0;
}

static int set_bucket_realloc(set s)
{
	struct set_type new_set;
	size_t prime = s->prime;
	float ratio = (float)((float)s->count / (float)s->capacity);

	if (prime > 0 && ratio <= SET_BUCKET_RATIO_MIN)
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
	new_set.count = 0;
	new_set.prime = prime;
	new_set.capacity = bucket_capacity(prime);
	new_set.buckets = bucket_create(new_set.capacity);

	if (new_set.buckets != NULL)
	{
		size_t iterator;

		/* Rehash all the elements into the new set */
		set_bucket_rehash(s, &new_set);

		/* Destroy all pairs from old set */
		for (iterator = 0; iterator < s->capacity; ++iterator)
		{
			bucket b = &s->buckets[iterator];

			if (b->pairs != NULL)
			{
				free(b->pairs);
			}
		}

		/* Destroy all buckets from old set */
		free(s->buckets);

		s->capacity = new_set.capacity;
		s->prime = new_set.prime;
		s->buckets = new_set.buckets;

		return 0;
	}

	return 1;
}

int set_insert(set s, set_key key, set_value value)
{
	set_hash h;
	size_t index;
	bucket b;
	pair p;

	if (s == NULL || key == NULL || value == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set insertion parameters");
		return 1;
	}

	h = s->hash_cb(key);

	index = h % s->capacity;

	b = &s->buckets[index];

	p = bucket_get_pair(b, s->compare_cb, key);

	if (p != NULL)
	{
		p->value = value;
		return 0;
	}

	if (bucket_insert(b, key, value) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set bucket insertion");
		return 1;
	}

	++s->count;

	return set_bucket_realloc(s);
}

int set_insert_array(set s, set_key keys[], set_value values[], size_t size)
{
	size_t iterator;

	if (s == NULL || keys == NULL || values == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set array insertion parameters");
		return 1;
	}

	for (iterator = 0; iterator < size; ++iterator)
	{
		if (set_insert(s, keys[iterator], values[iterator]) != 0)
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
		set_hash h = s->hash_cb(key);

		size_t index = h % s->capacity;

		bucket b = &s->buckets[index];

		pair p = bucket_get_pair(b, s->compare_cb, key);

		if (p != NULL)
		{
			return p->value;
		}
	}

	return NULL;
}

int set_contains(set s, set_key key)
{
	if (s != NULL && key != NULL)
	{
		set_hash h = s->hash_cb(key);

		size_t index = h % s->capacity;

		bucket b = &s->buckets[index];

		pair p = bucket_get_pair(b, s->compare_cb, key);

		if (p != NULL)
		{
			return 0;
		}
	}

	return 1;
}

int set_contains_any(set dest, set src)
{
	size_t bucket_iterator, pair_iterator;

	for (bucket_iterator = 0; bucket_iterator < src->capacity; ++bucket_iterator)
	{
		bucket b = &src->buckets[bucket_iterator];

		if (b->pairs != NULL && b->count > 0)
		{
			for (pair_iterator = 0; pair_iterator < b->count; ++pair_iterator)
			{
				pair p = &b->pairs[pair_iterator];

				if (set_contains(dest, p->key) == 0)
				{
					return 0;
				}
			}
		}
	}

	return 1;
}

int set_contains_which(set dest, set src, set_key *key)
{
	size_t bucket_iterator, pair_iterator;

	for (bucket_iterator = 0; bucket_iterator < src->capacity; ++bucket_iterator)
	{
		bucket b = &src->buckets[bucket_iterator];

		if (b->pairs != NULL && b->count > 0)
		{
			for (pair_iterator = 0; pair_iterator < b->count; ++pair_iterator)
			{
				pair p = &b->pairs[pair_iterator];

				if (set_contains(dest, p->key) == 0)
				{
					*key = p->key;
					return 0;
				}
			}
		}
	}

	return 1;
}

set_value set_remove(set s, set_key key)
{
	set_hash h;
	size_t index;
	bucket b;
	set_value value = NULL;

	if (s == NULL || key == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set remove parameters");
		return NULL;
	}

	h = s->hash_cb(key);

	index = h % s->capacity;

	b = &s->buckets[index];

	if (bucket_remove(b, s->compare_cb, key, &value) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set bucket remove: %p", key);
		return NULL;
	}

	--s->count;

	if (set_bucket_realloc(s) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set bucket remove reallocation");
		return NULL;
	}

	return value;
}

void set_iterate(set s, set_cb_iterate iterate_cb, set_cb_iterate_args args)
{
	if (s != NULL && s->buckets != NULL && iterate_cb != NULL)
	{
		size_t bucket_iterator;

		for (bucket_iterator = 0; bucket_iterator < s->capacity; ++bucket_iterator)
		{
			bucket b = &s->buckets[bucket_iterator];

			if (b->pairs != NULL && b->count > 0)
			{
				size_t pair_iterator;

				for (pair_iterator = 0; pair_iterator < b->count; ++pair_iterator)
				{
					pair p = &b->pairs[pair_iterator];

					if (iterate_cb(s, p->key, p->value, args) != 0)
					{
						return;
					}
				}
			}
		}
	}
}

int set_append(set dest, set src)
{
	size_t bucket_iterator, pair_iterator;

	for (bucket_iterator = 0; bucket_iterator < src->capacity; ++bucket_iterator)
	{
		bucket b = &src->buckets[bucket_iterator];

		if (b->pairs != NULL && b->count > 0)
		{
			for (pair_iterator = 0; pair_iterator < b->count; ++pair_iterator)
			{
				pair p = &b->pairs[pair_iterator];

				if (set_insert(dest, p->key, p->value) != 0)
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

int set_disjoint(set dest, set src)
{
	size_t bucket_iterator, pair_iterator;

	for (bucket_iterator = 0; bucket_iterator < src->capacity; ++bucket_iterator)
	{
		bucket b = &src->buckets[bucket_iterator];

		if (b->pairs != NULL && b->count > 0)
		{
			for (pair_iterator = 0; pair_iterator < b->count; ++pair_iterator)
			{
				pair p = &b->pairs[pair_iterator];

				set_value deleted = set_remove(dest, p->key);

				if (deleted != p->value)
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

int set_clear(set s)
{
	if (s == NULL)
	{
		return 1;
	}

	if (s->buckets != NULL)
	{
		size_t iterator;

		for (iterator = 0; iterator < s->capacity; ++iterator)
		{
			bucket b = &s->buckets[iterator];

			if (b->pairs != NULL)
			{
				free(b->pairs);
			}
		}

		free(s->buckets);
	}

	s->count = 0;
	s->prime = 0;
	s->capacity = bucket_capacity(s->prime);
	s->buckets = bucket_create(s->capacity);

	if (s->buckets == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Bad set clear bucket creation");
		return 1;
	}

	return 0;
}

void set_destroy(set s)
{
	if (s == NULL)
	{
		return;
	}

	if (s->buckets != NULL)
	{
		size_t iterator;

		for (iterator = 0; iterator < s->capacity; ++iterator)
		{
			bucket b = &s->buckets[iterator];

			if (b->pairs != NULL)
			{
				free(b->pairs);
			}
		}

		free(s->buckets);
	}

	free(s);
}

void set_iterator_begin(set_iterator it, set s)
{
	if (it != NULL)
	{
		it->current_bucket = 0;
		it->current_pair = 0;

		if (s != NULL && s->buckets != NULL && set_size(s) > 0)
		{
			it->s = s;

			set_iterator_next(it);
		}
		else
		{
			it->s = NULL;
		}
	}
}

set_key set_iterator_key(set_iterator it)
{
	if (it != NULL && it->s != NULL && it->current_bucket < it->s->capacity && it->current_pair > 0)
	{
		return it->s->buckets[it->current_bucket].pairs[it->current_pair - 1].key;
	}

	return NULL;
}

set_value set_iterator_value(set_iterator it)
{
	if (it != NULL && it->s != NULL && it->current_bucket < it->s->capacity && it->current_pair > 0)
	{
		return it->s->buckets[it->current_bucket].pairs[it->current_pair - 1].value;
	}

	return NULL;
}

void set_iterator_next(set_iterator it)
{
	if (it != NULL && it->s != NULL)
	{
		for (; it->current_bucket < it->s->capacity; ++it->current_bucket)
		{
			bucket b = &it->s->buckets[it->current_bucket];

			if (b->pairs != NULL && b->count > 0)
			{
				for (; it->current_pair < b->count; ++it->current_pair)
				{
					pair p = &b->pairs[it->current_pair];

					if (p != NULL)
					{
						++it->current_pair;

						return;
					}
				}

				it->current_pair = 0;
			}
		}
	}
}

int set_iterator_end(set_iterator it)
{
	if (it != NULL && it->s != NULL)
	{
		if (it->current_bucket >= it->s->capacity)
		{
			return 0;
		}

		return 1;
	}

	return 0;
}
