/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <adt/adt_set_small.h>

#include <log/log.h>

#include <string.h>

/* -- Definitions -- */

#define SET_SMALL_KEY_SIZE 8

/* -- Member Data -- */

union set_small_key_type
{
	char string[SET_SMALL_KEY_SIZE];
	uint64_t integer;
};

struct set_small_element_type
{
	union set_small_key_type key; /* Key value of 8 byte size for fast comparison */
	void *value;				  /* Actual data, owned by the caller */
};

struct set_small_type
{
	size_t capacity;					   /* Fixed value, total amount of allocated slots */
	size_t size;						   /* Current amount of elements in data */
	struct set_small_element_type data[1]; /* Contigously allocated memory */
};

/* -- Private Methods -- */

static set_small_element set_small_find(set_small s, union set_small_key_type set_key)
{
	size_t iterator;

	/* Find if the element already exists, use integer for comparing */
	for (iterator = 0; iterator < s->size; ++iterator)
	{
		set_small_element element = &s->data[iterator];

		if (set_key.integer == element->key.integer)
		{
			return element;
		}
	}

	return NULL;
}

/* -- Methods -- */

set_small set_small_create(size_t capacity)
{
	size_t array_size;
	set_small s;

	if (capacity == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set small create capacity");
		return NULL;
	}

	array_size = sizeof(struct set_small_element_type) * capacity;
	s = malloc(sizeof(struct set_small_type) - sizeof(struct set_small_element_type) + array_size);

	if (s == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set small create allocation");
		return NULL;
	}

	s->capacity = capacity;
	s->size = 0;
	memset(&s->data[0], 0, array_size);

	return s;
}

size_t set_small_capacity(set_small s)
{
	if (s == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set small capacity parameters");
		return 0;
	}

	return s->capacity;
}

size_t set_small_size(set_small s)
{
	if (s == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set small capacity parameters");
		return 0;
	}

	return s->size;
}

int set_small_insert(set_small s, const char *key, void *value)
{
	union set_small_key_type set_key = { 0 };
	set_small_element element = NULL;
	size_t key_size = 0;

	if (s == NULL || key == NULL || value == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set small insert parameters");
		return 1;
	}

	/* Copy string into key with last bytes set to 0 */
	key_size = strnlen(key, SET_SMALL_KEY_SIZE);
	memcpy(set_key.string, key, key_size);

	/* Find if the element already exists */
	element = set_small_find(s, set_key);

	if (element != NULL)
	{
		element->value = value;
		return 0;
	}

	/* Element not found in the array, insert it */
	element = &s->data[s->size++];
	element->key.integer = set_key.integer;
	element->value = value;

	return 0;
}

void *set_small_get(set_small s, const char *key)
{
	union set_small_key_type set_key = { 0 };
	set_small_element element = NULL;
	size_t key_size = 0;

	if (s == NULL || key == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set small get parameters");
		return NULL;
	}

	/* Copy string into key with last bytes set to 0 */
	key_size = strnlen(key, SET_SMALL_KEY_SIZE);
	memcpy(set_key.string, key, key_size);

	/* Find if the element already exists */
	element = set_small_find(s, set_key);

	if (element != NULL)
	{
		return element->value;
	}

	return NULL;
}

void *set_small_remove(set_small s, const char *key)
{
	union set_small_key_type set_key = { 0 };
	size_t iterator = 0;
	size_t key_size = 0;

	if (s == NULL || key == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set small remove parameters");
		return NULL;
	}

	/* Copy string into key with last bytes set to 0 */
	key_size = strnlen(key, SET_SMALL_KEY_SIZE);
	memcpy(set_key.string, key, key_size);

	/* Find the element and remove it */
	for (iterator = 0; iterator < s->size; ++iterator)
	{
		set_small_element element = &s->data[iterator];

		if (set_key.integer == element->key.integer)
		{
			void *value = element->value;

			memmove(&s->data[iterator], &s->data[iterator + 1], (s->size - iterator - 1) * sizeof(struct set_small_element_type));

			--s->size;

			return value;
		}
	}

	return NULL;
}

int set_small_clear(set_small s)
{
	if (s == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set small clear parameters");
		return 1;
	}

	s->size = 0;

	return 0;
}

void set_small_destroy(set_small s)
{
	if (s == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid set small destroy parameters");
		return;
	}

	free(s);
}

void set_small_iterator_begin(set_small_iterator it, set_small s)
{
	it->s = s;
	it->index = 0;
}

const char *set_small_iterator_key(set_small_iterator it)
{
	return it->s->data[it->index].key.string;
}

void *set_small_iterator_value(set_small_iterator it)
{
	return it->s->data[it->index].value;
}

void set_small_iterator_next(set_small_iterator it)
{
	++it->index;
}

int set_small_iterator_end(set_small_iterator it)
{
	return it->index < it->s->size;
}
