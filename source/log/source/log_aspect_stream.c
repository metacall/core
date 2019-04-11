/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_aspect_stream.h>
#include <log/log_policy_stream.h>

#include <log/log_aspect_schedule.h>
#include <log/log_policy_schedule.h>

#include <log/log_aspect_format.h>
#include <log/log_policy_format.h>

#include <log/log_record.h>
#include <log/log_impl.h>

/* -- Forward Declarations -- */

struct log_aspect_stream_execute_cb_data_type;

struct log_aspect_stream_write_cb_data_type;

/* -- Type Definitions -- */

typedef struct log_aspect_stream_execute_cb_data_type * log_aspect_stream_execute_cb_data;

typedef struct log_aspect_stream_write_cb_data_type * log_aspect_stream_write_cb_data;

/* -- Member Data -- */

struct log_aspect_stream_write_cb_data_type
{
	log_impl impl;
	log_record record;
};

struct log_aspect_stream_execute_cb_data_type
{
	log_impl impl;
	log_aspect aspect;
	log_record_ctor record_ctor;
};

/* -- Private Methods -- */

static log_aspect_data log_aspect_stream_create(log_aspect aspect, const log_aspect_ctor ctor);

static int log_aspect_stream_impl_write_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data);

static int log_aspect_stream_impl_write(log_aspect aspect, const log_record_ctor record_ctor);

static int log_aspect_stream_impl_flush_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data);

static int log_aspect_stream_impl_flush(log_aspect aspect);

static int log_aspect_stream_destroy(log_aspect aspect);

/* -- Methods -- */

log_aspect_interface log_aspect_stream_interface()
{
	static struct log_aspect_stream_impl_type log_aspect_stream_impl_obj =
	{
		&log_aspect_stream_impl_write,
		&log_aspect_stream_impl_flush
	};

	static struct log_aspect_interface_type aspect_interface_stream =
	{
		&log_aspect_stream_create,
		&log_aspect_stream_impl_obj,
		&log_aspect_stream_destroy
	};

	return &aspect_interface_stream;
}

static log_aspect_data log_aspect_stream_create(log_aspect aspect, const log_aspect_ctor ctor)
{
	/* TODO */
	(void)aspect;
	(void)ctor;

	return NULL;
}

static int log_aspect_stream_impl_write_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data)
{
	log_aspect_stream_write_cb_data write_args = notify_data;

	log_policy_stream_impl stream_impl = log_policy_derived(policy);

	log_aspect format = log_impl_aspect(write_args->impl, LOG_ASPECT_FORMAT);

	log_aspect_format_impl format_impl = log_aspect_derived(format);

	size_t size = format_impl->size(format, write_args->record);

	(void)aspect;

	if (size == 0)
	{
		return 1;
	}

	/* TODO: remove this, use storage policy instead */
	{
		void * buffer = malloc(size);

		int result = 1;

		if (buffer == NULL)
		{
			return 1;
		}

		if (format_impl->serialize(format, write_args->record, buffer, size) != 0)
		{
			free(buffer);

			return 1;
		}

		if (stream_impl->write(policy, buffer, size) == 0)
		{
			result = stream_impl->flush(policy);
		}

		free(buffer);

		return result;
	}
}

static int log_aspect_stream_impl_write_execute_cb(log_policy policy, log_aspect_schedule_data data)
{
	log_aspect_stream_execute_cb_data execute_data = data;

	log_policy_schedule_impl schedule_impl = log_policy_derived(policy);

	log_handle handle = log_impl_handle(execute_data->impl);

	log_record record;

	int result;

	struct log_aspect_stream_write_cb_data_type write_data;

	if (schedule_impl->lock(policy) != 0)
	{
		return 1;
	}

	record = log_handle_push(handle, execute_data->record_ctor);

	if (schedule_impl->unlock(policy) != 0)
	{
		return 1;
	}

	if (record == NULL)
	{
		return 1;
	}

	write_data.impl = execute_data->impl;
	write_data.record = record;

	result = log_aspect_notify_all(execute_data->aspect, &log_aspect_stream_impl_write_cb, (log_aspect_notify_data)&write_data);

	if (schedule_impl->lock(policy) != 0)
	{
		return 1;
	}

	log_handle_pop(handle);

	if (schedule_impl->unlock(policy) != 0)
	{
		return 1;
	}

	return result;
}

static int log_aspect_stream_impl_write(log_aspect aspect, const log_record_ctor record_ctor)
{
	log_impl impl = log_aspect_parent(aspect);

	log_aspect schedule = log_impl_aspect(impl, LOG_ASPECT_SCHEDULE);

	log_aspect_schedule_impl schedule_impl = log_aspect_derived(schedule);

	struct log_aspect_stream_execute_cb_data_type data;

	data.impl = impl;
	data.aspect = aspect;
	data.record_ctor = record_ctor;

	return schedule_impl->execute(schedule, &log_aspect_stream_impl_write_execute_cb, (log_aspect_schedule_data)&data);
}

static int log_aspect_stream_impl_flush_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data)
{
	log_policy_stream_impl stream_impl = log_policy_derived(policy);

	(void)aspect;
	(void)notify_data;

	return stream_impl->flush(policy);
}

static int log_aspect_stream_impl_flush(log_aspect aspect)
{
	return log_aspect_notify_all(aspect, &log_aspect_stream_impl_flush_cb, NULL);
}

static int log_aspect_stream_destroy(log_aspect aspect)
{
	/* TODO */
	(void)aspect;

	return 0;
}
