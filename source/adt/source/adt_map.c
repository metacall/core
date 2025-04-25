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
#include <adt/adt_map.h>

#include <log/log.h>

/* -- Definitions -- */

#define MAP_BUCKET_RATIO_MIN 0.1f
#define MAP_BUCKET_RATIO_MAX 0.77f

/* -- Member Data -- */

struct map_type
{
	size_t count;
	size_t capacity;
	size_t prime;
	bucket buckets;
	map_cb_hash hash_cb;
	map_cb_compare compare_cb;
};

struct map_iterator_type
{
	map m;
	size_t current_bucket;
	size_t current_pair;
};

struct map_contains_any_cb_iterator_type
{
	map m;
	int result;
};

typedef struct map_contains_any_cb_iterator_type *map_contains_any_cb_iterator;

/* -- Methods -- */

map map_create(map_cb_hash hash_cb, map_cb_compare compare_cb)
{
	map m;

	if (hash_cb == NULL || compare_cb == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid map creation parameters");
		return NULL;
	}

	m = malloc(sizeof(struct map_type));

	if (m == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Bad map allocation");
		return NULL;
	}

	m->hash_cb = hash_cb;
	m->compare_cb = compare_cb;
	m->count = 0;
	m->prime = 0;
	m->capacity = bucket_capacity(m->prime);
	m->buckets = bucket_create(m->capacity);

	if (m->buckets == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Bad map bucket creation");
		free(m);
		return NULL;
	}

	return m;
}

size_t map_size(map m)
{
	if (m != NULL)
	{
		return m->count;
	}

	return 0;
}

static int map_bucket_realloc_iterator(map m, map_key key, map_value value, map_cb_iterate_args args)
{
	map new_map = (map)args;

	if (new_map != m && key != NULL && args != NULL)
	{
		map_hash h = new_map->hash_cb(key);

		size_t index = h % new_map->capacity;

		bucket b = &new_map->buckets[index];

		if (bucket_insert(b, key, value) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid map bucket realloc insertion");
			return 1;
		}

		++new_map->count;

		return 0;
	}

	return 1;
}

static int map_bucket_realloc(map m)
{
	struct map_type new_map;
	size_t prime = m->prime;
	float ratio = (float)((float)m->count / (float)m->capacity);

	if (prime > 0 && ratio <= MAP_BUCKET_RATIO_MIN)
	{
		--prime;
	}
	else if (ratio >= MAP_BUCKET_RATIO_MAX)
	{
		++prime;
	}
	else
	{
		return 0;
	}

	new_map.hash_cb = m->hash_cb;
	new_map.compare_cb = m->compare_cb;
	new_map.count = 0;
	new_map.prime = prime;
	new_map.capacity = bucket_capacity(prime);
	new_map.buckets = bucket_create(new_map.capacity);

	if (new_map.buckets != NULL)
	{
		size_t iterator;

		map_iterate(m, &map_bucket_realloc_iterator, &new_map);

		for (iterator = 0; iterator < m->capacity; ++iterator)
		{
			bucket b = &m->buckets[iterator];

			if (b->pairs != NULL)
			{
				free(b->pairs);
			}
		}

		free(m->buckets);

		m->capacity = new_map.capacity;
		m->prime = new_map.prime;
		m->buckets = new_map.buckets;

		return 0;
	}

	return 1;
}

int map_insert(map m, map_key key, map_value value)
{
	map_hash h;
	size_t index;
	bucket b;

	if (m == NULL || key == NULL || value == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid map insertion parameters");
		return 1;
	}

	h = m->hash_cb(key);

	index = h % m->capacity;

	b = &m->buckets[index];

	if (bucket_insert(b, key, value) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid map bucket insertion");
		return 1;
	}

	++m->count;

	return map_bucket_realloc(m);
}

int map_insert_array(map m, map_key keys[], map_value values[], size_t size)
{
	size_t iterator;

	if (m == NULL || keys == NULL || values == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid map insertion parameters");
		return 1;
	}

	for (iterator = 0; iterator < size; ++iterator)
	{
		if (map_insert(m, keys[iterator], values[iterator]) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid map array insertion");
			return 1;
		}
	}

	return 0;
}

vector map_get(map m, map_key key)
{
	if (m != NULL && key != NULL)
	{
		map_hash h = m->hash_cb(key);

		size_t index = h % m->capacity;

		bucket b = &m->buckets[index];

		return bucket_get_pairs_value(b, m->compare_cb, key);
	}

	return NULL;
}

int map_contains(map m, map_key key)
{
	if (m != NULL && key != NULL)
	{
		map_hash h = m->hash_cb(key);

		size_t index = h % m->capacity;

		bucket b = &m->buckets[index];

		pair p = bucket_get_pair(b, m->compare_cb, key);

		if (p != NULL)
		{
			return 0;
		}
	}

	return 1;
}

static int map_contains_any_cb_iterate(map m, map_key key, map_value value, map_cb_iterate_args args)
{
	map_contains_any_cb_iterator iterator = (map_contains_any_cb_iterator)args;

	(void)m;
	(void)value;

	iterator->result = map_contains(iterator->m, key);

	/* Stop iteration if we found an element */
	return !iterator->result;
}

int map_contains_any(map dest, map src)
{
	struct map_contains_any_cb_iterator_type args;

	args.m = dest;
	args.result = 1;

	map_iterate(src, &map_contains_any_cb_iterate, (map_cb_iterate_args)&args);

	return args.result;
}

map_value map_remove(map m, map_key key)
{
	map_hash h;
	size_t index;
	bucket b;
	map_value value = NULL;

	if (m == NULL || key == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid map remove parameters");
		return NULL;
	}

	h = m->hash_cb(key);

	index = h % m->capacity;

	b = &m->buckets[index];

	if (bucket_remove(b, m->compare_cb, key, &value) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid map bucket remove: %p", key);
		return NULL;
	}

	--m->count;

	if (map_bucket_realloc(m) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid map bucket remove reallocation");
		return NULL;
	}

	return value;
}

vector map_remove_all(map m, map_key key)
{
	map_hash h;
	size_t index, iterator, size;
	bucket b;
	vector v = NULL;

	if (m == NULL || key == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid map remove parameters");
		return NULL;
	}

	h = m->hash_cb(key);

	index = h % m->capacity;

	b = &m->buckets[index];

	v = map_get(m, key);

	size = vector_size(v);

	if (size == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid map bucket remove: %p", key);
		vector_destroy(v);
		return NULL;
	}

	for (iterator = 0; iterator < size; ++iterator)
	{
		if (bucket_remove(b, m->compare_cb, key, NULL) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid map bucket remove: %p", key);
			vector_destroy(v);
			return NULL;
		}

		--m->count;
	}

	if (map_bucket_realloc(m) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid map bucket remove reallocation");
		vector_destroy(v);
		return NULL;
	}

	return v;
}

void map_iterate(map m, map_cb_iterate iterate_cb, map_cb_iterate_args args)
{
	if (m != NULL && m->buckets != NULL && iterate_cb != NULL)
	{
		size_t bucket_iterator;

		for (bucket_iterator = 0; bucket_iterator < m->capacity; ++bucket_iterator)
		{
			bucket b = &m->buckets[bucket_iterator];

			if (b->pairs != NULL && b->count > 0)
			{
				size_t pair_iterator;

				for (pair_iterator = 0; pair_iterator < b->count; ++pair_iterator)
				{
					pair p = &b->pairs[pair_iterator];

					if (iterate_cb(m, p->key, p->value, args) != 0)
					{
						return;
					}
				}
			}
		}
	}
}

static int map_append_cb_iterate(map m, map_key key, map_value value, map_cb_iterate_args args)
{
	map dest = (map)args;

	(void)m;

	return map_insert(dest, key, value);
}

int map_append(map dest, map src)
{
	map_cb_iterate_args args = (map_cb_iterate_args)dest;

	map_iterate(src, &map_append_cb_iterate, args);

	return 0;
}

int map_clear(map m)
{
	if (m == NULL)
	{
		return 1;
	}

	if (m->buckets != NULL)
	{
		size_t iterator;

		for (iterator = 0; iterator < m->capacity; ++iterator)
		{
			bucket b = &m->buckets[iterator];

			if (b->pairs != NULL)
			{
				free(b->pairs);
			}
		}

		free(m->buckets);
	}

	m->count = 0;
	m->prime = 0;
	m->capacity = bucket_capacity(m->prime);
	m->buckets = bucket_create(m->capacity);

	if (m->buckets == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Bad map clear bucket creation");
		return 1;
	}

	return 0;
}

void map_destroy(map m)
{
	if (m == NULL)
	{
		return;
	}

	if (m->buckets != NULL)
	{
		size_t iterator;

		for (iterator = 0; iterator < m->capacity; ++iterator)
		{
			bucket b = &m->buckets[iterator];

			if (b->pairs != NULL)
			{
				free(b->pairs);
			}
		}

		free(m->buckets);
	}

	free(m);
}

map_iterator map_iterator_begin(map m)
{
	if (m != NULL && m->buckets != NULL && map_size(m) > 0)
	{
		map_iterator it = malloc(sizeof(struct map_iterator_type));

		if (it != NULL)
		{
			it->m = m;
			it->current_bucket = 0;
			it->current_pair = 0;

			map_iterator_next(it);

			return it;
		}
	}

	return NULL;
}

map_key map_iterator_get_key(map_iterator it)
{
	if (it != NULL && it->current_bucket < it->m->capacity && it->current_pair > 0)
	{
		return it->m->buckets[it->current_bucket].pairs[it->current_pair - 1].key;
	}

	return NULL;
}

map_value map_iterator_get_value(map_iterator it)
{
	if (it != NULL && it->current_bucket < it->m->capacity && it->current_pair > 0)
	{
		return it->m->buckets[it->current_bucket].pairs[it->current_pair - 1].value;
	}

	return NULL;
}

void map_iterator_next(map_iterator it)
{
	if (it != NULL)
	{
		for (; it->current_bucket < it->m->capacity; ++it->current_bucket)
		{
			bucket b = &it->m->buckets[it->current_bucket];

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
			}
		}
	}
}

int map_iterator_end(map_iterator *it)
{
	if (it != NULL && *it != NULL)
	{
		if ((*it)->current_bucket >= (*it)->m->capacity)
		{
			free(*it);

			*it = NULL;

			return 0;
		}

		return 1;
	}

	return 0;
}
