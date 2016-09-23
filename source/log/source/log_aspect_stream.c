/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_aspect_stream.h>
#include <log/log_policy_stream.h>

/* -- Member Data -- */

struct log_aspect_stream_write_type
{
	const void * buffer;
	size_t size;
};

/* -- Private Methods -- */

LOG_NO_EXPORT static log_aspect_data log_aspect_stream_create(log_aspect aspect, const log_aspect_ctor ctor);

LOG_NO_EXPORT static int log_aspect_stream_impl_write_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data);

LOG_NO_EXPORT static int log_aspect_stream_impl_write(log_aspect aspect, const void * buffer, const size_t size);

LOG_NO_EXPORT static int log_aspect_stream_impl_flush_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data);

LOG_NO_EXPORT static int log_aspect_stream_impl_flush(log_aspect aspect);

LOG_NO_EXPORT static int log_aspect_stream_destroy(log_aspect aspect);

/* -- Methods -- */

log_aspect_interface log_aspect_stream_interface()
{
	static struct log_aspect_stream_impl_type log_aspect_stream_impl =
	{
		&log_aspect_stream_impl_write,
		&log_aspect_stream_impl_flush
	};

	static struct log_aspect_interface_type aspect_interface_stream =
	{
		&log_aspect_stream_create,
		&log_aspect_stream_impl,
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
	struct log_aspect_stream_write_type * write_args = notify_data;

	log_policy_stream_impl stream_impl = log_policy_derived(policy);

	(void)aspect;

	return stream_impl->write(policy, write_args->buffer, write_args->size);
}

static int log_aspect_stream_impl_write(log_aspect aspect, const void * buffer, const size_t size)
{
	struct log_aspect_stream_write_type notify_data;
	
	notify_data.buffer = buffer;
	notify_data.size = size;

	return log_aspect_notify_all(aspect, &log_aspect_stream_impl_write_cb, (log_aspect_notify_data)&notify_data);
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
