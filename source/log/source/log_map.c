/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#include <log/log_map.h>
#include <log/log_valid_size.h>
#include <log/log_impl.h>

#include <string.h>

/* -- Definitions -- */

#define LOG_MAP_BUCKET_SIZE ((size_t)0x00000100)

/* -- Forward Declarations -- */

struct log_map_bucket_type;

/* -- Type Definitions -- */

typedef struct log_map_bucket_type * log_map_bucket;

/* -- Member Data -- */

struct log_map_bucket_type
{
	const char * key;
	const void * value;
	log_map_bucket next;
};

struct log_map_type
{
	struct log_map_table_type
	{
		log_map_bucket data;	/**< Hash table pointer */
		size_t count;			/**< Number of unique key-value pairs introduced */
		size_t size;			/**< Fixed capacity of the hash table */
	} table;

	struct log_map_block_type
	{
		log_map_bucket * data;	/**< Block storage holding arrays of buckets */
		size_t count;			/**< Number of blocks used in the storage */
		size_t size;			/**< Fixed capacity of the storage */
		size_t position;		/**< Current block bucket iterator */
	} block;
};

struct log_map_iterator_type
{
	log_map map;
	log_map_bucket bucket;
	log_map_bucket next;
	size_t position;
};

/* -- Private Methods -- */

static size_t log_map_hash_fnv1(const char * key)
{
	size_t hash = ((size_t)0x811C9DC5);

	const char * iterator = key;

	while (*iterator != '\0')
	{
		hash = (((size_t)0x01000193) * hash) ^ (*iterator);

		++iterator;
	}

	return hash;
}

/* -- Methods -- */

log_map log_map_create(size_t size)
{
	size_t iterator;

	log_map map = NULL;

	size = log_valid_size(size);

	if (size < LOG_MAP_MIN_SIZE)
	{
		size = LOG_MAP_MIN_SIZE;
	}
	else if (size > LOG_MAP_MAX_SIZE)
	{
		size = LOG_MAP_MAX_SIZE;
	}

	map = malloc(sizeof(struct log_map_type));

	if (map == NULL)
	{
		return NULL;
	}

	map->table.data = malloc(sizeof(struct log_map_bucket_type) * size);

	if (map->table.data == NULL)
	{
		free(map);

		return NULL;
	}

	map->table.count = 0;
	map->table.size = size;

	for (iterator = 0; iterator < map->table.size; ++iterator)
	{
		log_map_bucket bucket = &map->table.data[iterator];

		bucket->key = NULL;
		bucket->value = NULL;
		bucket->next = NULL;
	}

	map->block.data = malloc(sizeof(log_map_bucket) * size);

	if (map->block.data == NULL)
	{
		free(map->table.data);

		free(map);

		return NULL;
	}

	map->block.count = 0;
	map->block.size = size;
	map->block.position = LOG_MAP_BUCKET_SIZE;

	for (iterator = 0; iterator < map->block.size; ++iterator)
	{
		map->block.data[iterator] = NULL;
	}

	return map;
}

size_t log_map_size(log_map map)
{
	return map->table.count;
}

size_t log_map_collisions(log_map map)
{
	return map->block.count;
}

int log_map_insert(log_map map, const char * key, const void * value)
{
	size_t hash = log_map_hash_fnv1(key) & (map->table.size - 1);

	log_map_bucket head = &map->table.data[hash];

	log_map_bucket bucket = head;

	if (head->key != NULL)
	{
		do
		{
			if (strcmp(bucket->key, key) == 0)
			{
				bucket->value = value;

				return 0;
			}

			bucket = bucket->next;

		} while (bucket != NULL);
	}

	++map->table.count;

	if (head->key == NULL)
	{
		head->key = key;
		head->value = value;
		head->next = NULL;

		return 0;
	}

	if (map->block.position >= LOG_MAP_BUCKET_SIZE)
	{
		log_map_bucket buckets = NULL;

		if (map->block.count >= map->block.size)
		{
			return 1;
		}

		buckets = malloc(sizeof(struct log_map_bucket_type) * LOG_MAP_BUCKET_SIZE);

		if (buckets == NULL)
		{
			return 1;
		}

		map->block.data[map->block.count] = buckets;

		map->block.position = 0;

		++map->block.count;
	}

	bucket = map->block.data[map->block.count - 1] + map->block.position;

	bucket->key = key;
	bucket->value = value;
	bucket->next = head->next;
	head->next = bucket;

	++map->block.position;

	return 0;
}

const void * log_map_get(log_map map, const char * key)
{
	size_t hash = log_map_hash_fnv1(key) & (map->table.size - 1);

	log_map_bucket bucket = &map->table.data[hash];

	if (bucket->key != NULL)
	{
		do
		{
			if (strcmp(bucket->key, key) == 0)
			{
				return bucket->value;
			}

			bucket = bucket->next;

		} while (bucket != NULL);
	}

	return NULL;
}

const void * log_map_remove(log_map map, const char * key)
{
	/* TODO: remove log impl from map and clear bucket */
	(void)map;
	(void)key;

	return NULL;
}

int log_map_clear(log_map map)
{
	/* TODO: clear all buckets and blocks */
	(void)map;

	return 0;
}

int log_map_destroy(log_map map)
{
	size_t iterator;

	if (map == NULL)
	{
		return 1;
	}
	
	if (map->table.data != NULL)
	{
		free(map->table.data);
	}

	if (map->block.data != NULL)
	{
		for (iterator = 0; iterator < map->block.count; ++iterator)
		{
			if (map->block.data[iterator] != NULL)
			{
				free(map->block.data[iterator]);
			}
		}

		free(map->block.data);
	}

	free(map);

	return 0;
}

log_map_iterator log_map_iterator_begin(log_map map)
{
	log_map_iterator iterator = NULL;

	if (map == NULL)
	{
		return NULL;
	}

	iterator = malloc(sizeof(struct log_map_iterator_type));

	if (iterator == NULL)
	{
		return NULL;
	}

	iterator->map = map;
	iterator->position = 0;
	iterator->bucket = NULL;
	iterator->next = &iterator->map->table.data[iterator->position];

	if (log_map_iterator_next(iterator) != 0)
	{
		free(iterator);

		return NULL;
	}

	return iterator;
}

const char * log_map_iterator_key(log_map_iterator iterator)
{
	if (iterator == NULL)
	{
		return NULL;
	}

	return iterator->bucket->key;
}

const void * log_map_iterator_value(log_map_iterator iterator)
{
	if (iterator == NULL)
	{
		return NULL;
	}

	return iterator->bucket->value;
}

int log_map_iterator_next(log_map_iterator iterator)
{
	if (iterator == NULL)
	{
		return 1;
	}

	iterator->bucket = iterator->next;

	while (iterator->position < iterator->map->table.size)
	{
		while (iterator->bucket != NULL)
		{
			if (iterator->bucket->key != NULL && iterator->bucket->value != NULL)
			{
				iterator->next = iterator->bucket->next;

				return 0;
			}

			iterator->bucket = iterator->bucket->next;
		}

		++iterator->position;

		iterator->bucket = &iterator->map->table.data[iterator->position];
	}
	
	return 1;
}

int log_map_iterator_end(log_map_iterator iterator)
{
	if (iterator == NULL)
	{
		return 1;
	}
	
	if (iterator->position < iterator->map->table.size && iterator->bucket != NULL)
	{
		return 1;
	}

	free(iterator);

	return 0;
}
