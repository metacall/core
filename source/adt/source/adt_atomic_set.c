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

#include <adt/adt_atomic_set.h>

#include <log/log.h>

#include <threading/threading_atomic.h>

#include <string.h>

/* -- Definitions -- */

#define ATOMIC_SET_KEY_SIZE 8

/* -- Member Data -- */

union atomic_set_key_type
{
	char string[ATOMIC_SET_KEY_SIZE];
	uint64_t integer;
};

struct atomic_set_element_type
{
	union atomic_set_key_type key; /* Key value of 8 byte size for fast comparison */
	void *value;				   /* Actual data, owned by the caller */
};

struct atomic_set_storage_type
{
	atomic_int_fast64_t ref_count;			/* Amount of references to this storage */
	size_t capacity;						/* Fixed value, total amount of allocated slots */
	size_t size;							/* Current amount of elements in data */
	struct atomic_set_element_type data[1]; /* Contigously allocated memory */
};

struct atomic_set_type
{
	_Atomic(atomic_set_storage) storage; /* Atomic pointer for implementing copy on write algorithm */
};

struct atomic_set_insert_type
{
	const char *key;
	void *value;
	int result;
};

/* -- Private Methods -- */

static atomic_set_storage atomic_set_storage_create(size_t capacity)
{
	size_t size = sizeof(struct atomic_set_element_type) * capacity;
	struct atomic_set_storage_type *storage = malloc(sizeof(struct atomic_set_storage_type) - sizeof(struct atomic_set_element_type) + size);

	if (storage == NULL)
	{
		return NULL;
	}

	atomic_init(&storage->ref_count, 1);
	storage->capacity = capacity;
	storage->size = 0;
	memset(&storage->data[0], 0, size);

	return storage;
}

static atomic_set_storage atomic_set_storage_copy(atomic_set_storage storage)
{
	size_t size = sizeof(struct atomic_set_element_type) * storage->capacity;
	struct atomic_set_storage_type *storage_copy = malloc(sizeof(struct atomic_set_storage_type) - sizeof(struct atomic_set_element_type) + size);

	if (storage_copy == NULL)
	{
		return NULL;
	}

	atomic_init(&storage_copy->ref_count, 1);
	storage_copy->capacity = storage->capacity;
	storage_copy->size = storage->size;
	memcpy(&storage_copy->data[0], &storage->data[0], size);

	return storage_copy;
}

static void atomic_set_storage_destroy(atomic_set_storage storage)
{
	if (storage != NULL && atomic_fetch_sub_explicit(&storage->ref_count, 1, memory_order_acq_rel) == 1)
	{
		free(storage);
	}
}

static const struct atomic_set_element_type *atomic_set_read(atomic_set s, atomic_set_storage *storage)
{
	atomic_set_storage current;

	for (;;)
	{
		/* Get current storage */
		current = atomic_load_explicit(&s->storage, memory_order_acquire);

		if (current == NULL)
		{
			*storage = NULL;
			return NULL;
		}

		/* Optimistically increment reference counter */
		atomic_fetch_add_explicit(&current->ref_count, 1, memory_order_relaxed);

		/* If no write was done in between, return it */
		if (current == atomic_load_explicit(&s->storage, memory_order_relaxed))
		{
			*storage = current;
			return current->data;
		}

		/* Some write was done in between, undo the operation */
		atomic_fetch_sub_explicit(&current->ref_count, 1, memory_order_relaxed);
	}
}

static void atomic_set_write(atomic_set s, void (*mutate)(atomic_set_storage, void *), void *context)
{
	atomic_set_storage current = atomic_load_explicit(&s->storage, memory_order_acquire);
	atomic_set_storage updated = NULL;

	do
	{
		/* Clean up from a previously failed CAS iteration */
		if (updated)
		{
			free(updated);
		}

		/* Copy the current storage */
		updated = atomic_set_storage_copy(current);

		/* Mutate the local clone completely hidden from other threads */
		mutate(updated, context);

		/* Attempt to atomically swing the root pointer to the updated block */
	} while (!atomic_compare_exchange_strong_explicit(&s->storage, &current, updated, memory_order_release, memory_order_acquire));

	/* The swap succeeded, release the writer's ownership stake of the old block */
	atomic_set_storage_destroy(current);
}

static void atomic_set_insert_impl(atomic_set_storage storage, void *context)
{
	struct atomic_set_insert_type *insert_ctx = (struct atomic_set_insert_type *)context;

	if (storage->size == storage->capacity)
	{
		insert_ctx->result = 1;
		return;
	}

	strncpy(storage->data[storage->size].key.string, insert_ctx->key, ATOMIC_SET_KEY_SIZE);
	storage->data[storage->size].value = insert_ctx->value;

	++storage->size;
}

/* -- Methods -- */

atomic_set atomic_set_create(size_t capacity)
{
	atomic_set s;
	struct atomic_set_storage_type *storage;

	if (capacity == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid atomic set creation size");
		return NULL;
	}

	s = malloc(sizeof(struct atomic_set_type));

	if (s == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Bad atomic set allocation");
		goto set_alloc_error;
	}

	storage = atomic_set_storage_create(capacity);

	if (storage == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Bad atomic set storage allocation");
		goto set_storage_alloc_error;
	}

	atomic_init(&s->storage, storage);

	return s;

set_storage_alloc_error:
	free(s);
set_alloc_error:
	return NULL;
}

size_t atomic_set_capacity(atomic_set s)
{
	size_t capacity = 0;
	atomic_set_storage storage = NULL;

	if (s == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid atomic set capacity");
		return 0;
	}

	if (atomic_set_read(s, &storage) == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid atomic set capacity read");
		return 0;
	}

	capacity = storage->capacity;

	atomic_set_storage_destroy(storage);

	return capacity;
}

size_t atomic_set_size(atomic_set s)
{
	size_t size = 0;
	atomic_set_storage storage = NULL;

	if (s == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid atomic set size");
		return 0;
	}

	if (atomic_set_read(s, &storage) == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid atomic set size read");
		return 0;
	}

	size = storage->size;

	atomic_set_storage_destroy(storage);

	return size;
}

int atomic_set_insert(atomic_set s, const char *key, void *value)
{
	struct atomic_set_insert_type context = { key, value, 0 };

	if (s == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid atomic set insert parameters");
		return 1;
	}

	atomic_set_write(s, &atomic_set_insert_impl, &context);

	return context.result;
}

void *atomic_set_get(atomic_set s, const char *key)
{
	atomic_set_storage storage = NULL;
	union atomic_set_key_type set_key = { 0 };
	void *value = NULL;
	size_t iterator = 0;

	if (s == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid atomic set get parameters");
		return NULL;
	}

	if (atomic_set_read(s, &storage) == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid atomic set get read");
		return NULL;
	}

	strncpy(set_key.string, key, ATOMIC_SET_KEY_SIZE);

	for (iterator = 0; iterator < storage->size; ++iterator)
	{
		if (storage->data[iterator].key.integer == set_key.integer)
		{
			value = storage->data[iterator].value;
			break;
		}
	}

	atomic_set_storage_destroy(storage);

	return value;
}

void atomic_set_destroy(atomic_set s)
{
	atomic_set_storage storage = atomic_load_explicit(&s->storage, memory_order_relaxed);
	atomic_set_storage_destroy(storage);
}
