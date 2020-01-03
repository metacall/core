/*
*	Logger Library by Parra Studios
*	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A generic logger library providing application execution reports.
*
*/

/* -- Headers -- */

#include <log/log_policy_stream_stdio.h>
#include <log/log_policy_stream.h>

/* -- Forward Declarations -- */

struct log_policy_stream_stdio_data_type;

/* -- Type Definitions -- */

typedef struct log_policy_stream_stdio_data_type * log_policy_stream_stdio_data;

/* -- Member Data -- */

struct log_policy_stream_stdio_data_type
{
	FILE * stream;
};

/* -- Private Methods -- */

static int log_policy_stream_stdio_create(log_policy policy, const log_policy_ctor ctor);

static int log_policy_stream_stdio_write(log_policy policy, const void * buffer, const size_t size);

static int log_policy_stream_stdio_flush(log_policy policy);

static int log_policy_stream_stdio_destroy(log_policy policy);

/* -- Methods -- */

log_policy_interface log_policy_stream_stdio_interface()
{
	static struct log_policy_stream_impl_type log_policy_stream_stdio_impl_obj =
	{
		&log_policy_stream_stdio_write,
		&log_policy_stream_stdio_flush
	};

	static struct log_policy_interface_type policy_interface_stream =
	{
		&log_policy_stream_stdio_create,
		&log_policy_stream_stdio_impl_obj,
		&log_policy_stream_stdio_destroy
	};

	return &policy_interface_stream;
}

static int log_policy_stream_stdio_create(log_policy policy, const log_policy_ctor ctor)
{
	log_policy_stream_stdio_data stdio_data = malloc(sizeof(struct log_policy_stream_stdio_data_type));

	const log_policy_stream_stdio_ctor stdio_ctor = ctor;

	if (stdio_data == NULL)
	{
		return 1;
	}

	stdio_data->stream = stdio_ctor->stream;

	log_policy_instantiate(policy, stdio_data, LOG_POLICY_STREAM_STDIO);

	return 0;
}

static int log_policy_stream_stdio_write(log_policy policy, const void * buffer, const size_t size)
{
	log_policy_stream_stdio_data stdio_data = log_policy_instance(policy);

	size_t length = size > 0 ? size - 1 : 0;

	/* Do not write null character */
	if (fwrite(buffer, 1, length, stdio_data->stream) != length)
	{
		return 1;
	}

	return 0;
}

static int log_policy_stream_stdio_flush(log_policy policy)
{
	log_policy_stream_stdio_data stdio_data = log_policy_instance(policy);

	return fflush(stdio_data->stream);
}

static int log_policy_stream_stdio_destroy(log_policy policy)
{
	log_policy_stream_stdio_data stdio_data = log_policy_instance(policy);

	if (stdio_data != NULL)
	{
		free(stdio_data);
	}

	return 0;
}
