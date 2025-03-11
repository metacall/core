/*
*	Logger Library by Parra Studios
*	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A generic logger library providing application execution reports.
*
*/

/* -- Headers -- */

#include <log/log_policy_stream.h>
#include <log/log_policy_stream_file.h>

/* -- Forward Declarations -- */

struct log_policy_stream_file_data_type;

/* -- Type Definitions -- */

typedef struct log_policy_stream_file_data_type *log_policy_stream_file_data;

/* -- Member Data -- */

struct log_policy_stream_file_data_type
{
	FILE *file;
};

/* -- Private Methods -- */

static int log_policy_stream_file_create(log_policy policy, const log_policy_ctor ctor);

static int log_policy_stream_file_write(log_policy policy, const void *buffer, const size_t size);

static int log_policy_stream_file_flush(log_policy policy);

static int log_policy_stream_file_destroy(log_policy policy);

/* -- Methods -- */

log_policy_interface log_policy_stream_file_interface(void)
{
	static struct log_policy_stream_impl_type log_policy_stream_file_impl_obj = {
		&log_policy_stream_file_write,
		&log_policy_stream_file_flush
	};

	static struct log_policy_interface_type policy_interface_stream = {
		&log_policy_stream_file_create,
		&log_policy_stream_file_impl_obj,
		&log_policy_stream_file_destroy
	};

	return &policy_interface_stream;
}

static int log_policy_stream_file_create(log_policy policy, const log_policy_ctor ctor)
{
	log_policy_stream_file_data file_data = malloc(sizeof(struct log_policy_stream_file_data_type));

	const log_policy_stream_file_ctor file_ctor = ctor;

	if (file_data == NULL)
	{
		return 1;
	}

	file_data->file = fopen(file_ctor->file_name, file_ctor->mode);

	log_policy_instantiate(policy, file_data, LOG_POLICY_STREAM_FILE);

	return 0;
}

static int log_policy_stream_file_write(log_policy policy, const void *buffer, const size_t size)
{
	log_policy_stream_file_data file_data = log_policy_instance(policy);

	size_t length = size > 0 ? size - 1 : 0;

	/* Do not write null character */
	if (fwrite(buffer, 1, length, file_data->file) != length)
	{
		return 1;
	}

	return 0;
}

static int log_policy_stream_file_flush(log_policy policy)
{
	log_policy_stream_file_data file_data = log_policy_instance(policy);

	return fflush(file_data->file);
}

static int log_policy_stream_file_destroy(log_policy policy)
{
	log_policy_stream_file_data file_data = log_policy_instance(policy);

	if (file_data != NULL)
	{
		fclose(file_data->file);

		free(file_data);
	}

	return 0;
}
