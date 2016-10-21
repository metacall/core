/*
*	Logger Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A generic logger library providing application execution reports.
*
*/

/* -- Headers -- */

#include <log/log_policy_storage_batch.h>
#include <log/log_policy_storage.h>

/* -- Definitions -- */

#define LOG_POLICY_STORAGE_BATCH_MIN_SIZE	((size_t)0x00000200)
#define LOG_POLICY_STORAGE_BATCH_MAX_SIZE	((size_t)0x00010000)

/* -- Forward Declarations -- */

struct log_policy_storage_batch_data_type;

/* -- Type Definitions -- */

typedef struct log_policy_storage_batch_data_type * log_policy_storage_batch_data;

/* -- Member Data -- */

struct log_policy_storage_batch_data_type
{
	void * buffer;
	size_t count;
	size_t size;
};

/* -- Private Methods -- */

static int log_policy_storage_batch_create(log_policy policy, const log_policy_ctor ctor);

static int log_policy_storage_batch_append(log_policy policy, const log_record record);

static int log_policy_storage_batch_flush(log_policy policy);

static int log_policy_storage_batch_destroy(log_policy policy);

/* -- Methods -- */

log_policy_interface log_policy_storage_batch_interface()
{
	static struct log_policy_storage_impl_type log_policy_storage_batch_impl_obj =
	{
		&log_policy_storage_batch_append,
		&log_policy_storage_batch_flush
	};

	static struct log_policy_interface_type policy_interface_storage =
	{
		&log_policy_storage_batch_create,
		&log_policy_storage_batch_impl_obj,
		&log_policy_storage_batch_destroy
	};

	return &policy_interface_storage;
}

static int log_policy_storage_batch_create(log_policy policy, const log_policy_ctor ctor)
{
	log_policy_storage_batch_data batch_data = malloc(sizeof(struct log_policy_storage_batch_data_type));

	const log_policy_storage_batch_ctor batch_ctor = ctor;

	if (batch_data == NULL)
	{
		return 1;
	}

	batch_data->count = 0;

	if (batch_ctor != NULL && batch_ctor->size >= LOG_POLICY_STORAGE_BATCH_MIN_SIZE && batch_ctor->size <= LOG_POLICY_STORAGE_BATCH_MAX_SIZE)
	{
		batch_data->size = batch_ctor->size;
	}
	else
	{
		batch_data->size = LOG_POLICY_STORAGE_BATCH_MIN_SIZE;
	}

	batch_data->buffer = malloc(batch_data->size);

	if (batch_data->buffer == NULL)
	{
		free(batch_data);

		return 1;
	}

	log_policy_instantiate(policy, batch_data, LOG_POLICY_STORAGE_BATCH);

	return 0;
}

static int log_policy_storage_batch_append(log_policy policy, const log_record record)
{
	(void)policy;
	(void)record;

	/* TODO */

	return 0;
}

static int log_policy_storage_batch_flush(log_policy policy)
{
	(void)policy;

	/* TODO */

	return 0;
}

static int log_policy_storage_batch_destroy(log_policy policy)
{
	log_policy_storage_batch_data batch_data = log_policy_instance(policy);

	if (batch_data != NULL)
	{
		if (batch_data->buffer != NULL)
		{
			free(batch_data->buffer);
		}

		free(batch_data);
	}

	return 0;
}
