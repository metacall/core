/*
*	Logger Library by Parra Studios
*	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A generic logger library providing application execution reports.
*
*/

/* -- Headers -- */

#include <log/log_policy_stream_custom.h>
#include <log/log_policy_stream.h>

/* -- Forward Declarations -- */

struct log_policy_stream_custom_data_type;

/* -- Type Definitions -- */

typedef struct log_policy_stream_custom_data_type * log_policy_stream_custom_data;

/* -- Member Data -- */

struct log_policy_stream_custom_data_type
{
	void * context;
	int (*stream_write)(void *, const char *, const size_t);
	int (*stream_flush)(void *);
};

/* -- Private Methods -- */

static int log_policy_stream_custom_create(log_policy policy, const log_policy_ctor ctor);

static int log_policy_stream_custom_write(log_policy policy, const void * buffer, const size_t size);

static int log_policy_stream_custom_flush(log_policy policy);

static int log_policy_stream_custom_destroy(log_policy policy);

/* -- Methods -- */

log_policy_interface log_policy_stream_custom_interface()
{
	static struct log_policy_stream_impl_type log_policy_stream_custom_impl_obj =
	{
		&log_policy_stream_custom_write,
		&log_policy_stream_custom_flush
	};

	static struct log_policy_interface_type policy_interface_stream =
	{
		&log_policy_stream_custom_create,
		&log_policy_stream_custom_impl_obj,
		&log_policy_stream_custom_destroy
	};

	return &policy_interface_stream;
}

static int log_policy_stream_custom_create(log_policy policy, const log_policy_ctor ctor)
{
	log_policy_stream_custom_data custom_data = NULL;

	const log_policy_stream_custom_ctor custom_ctor = ctor;

	if (custom_ctor->stream_write == NULL || custom_ctor->stream_flush == NULL)
	{
		return 1;
	}

	custom_data = malloc(sizeof(struct log_policy_stream_custom_data_type));

	if (custom_data == NULL)
	{
		return 1;
	}

	custom_data->context = custom_ctor->context;
	custom_data->stream_write = custom_ctor->stream_write;
	custom_data->stream_flush = custom_ctor->stream_flush;

	log_policy_instantiate(policy, custom_data, LOG_POLICY_STREAM_CUSTOM);

	return 0;
}

static int log_policy_stream_custom_write(log_policy policy, const void * buffer, const size_t size)
{
	log_policy_stream_custom_data custom_data = log_policy_instance(policy);

	return custom_data->stream_write(custom_data->context, (const char *)buffer, size);
}

static int log_policy_stream_custom_flush(log_policy policy)
{
	log_policy_stream_custom_data custom_data = log_policy_instance(policy);

	return custom_data->stream_flush(custom_data->context);
}

static int log_policy_stream_custom_destroy(log_policy policy)
{
	log_policy_stream_custom_data custom_data = log_policy_instance(policy);

	if (custom_data != NULL)
	{
		free(custom_data);
	}

	return 0;
}
