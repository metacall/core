/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_aspect_format.h>
#include <log/log_policy_format.h>

/* -- Member Data -- */

struct log_aspect_format_size_type
{
	log_record record;
	size_t size;
};

struct log_aspect_format_serialize_type
{
	log_record record;
	void * buffer;
	size_t size;
};

struct log_aspect_format_deserialize_type
{
	log_record record;
	const void * buffer;
	size_t size;
};

/* -- Private Methods -- */

static log_aspect_data log_aspect_format_create(log_aspect aspect, const log_aspect_ctor ctor);

static int log_aspect_format_impl_size_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data);

static size_t log_aspect_format_impl_size(log_aspect aspect, log_record record);

static int log_aspect_format_impl_serialize_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data);

static int log_aspect_format_impl_serialize(log_aspect aspect, log_record record, void * buffer, const size_t size);

static int log_aspect_format_impl_deserialize_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data);

static int log_aspect_format_impl_deserialize(log_aspect aspect, log_record record, const void * buffer, const size_t size);

static int log_aspect_format_destroy(log_aspect aspect);

/* -- Methods -- */

log_aspect_interface log_aspect_format_interface()
{
	static struct log_aspect_format_impl_type log_aspect_format_impl_obj =
	{
		&log_aspect_format_impl_size,
		&log_aspect_format_impl_serialize,
		&log_aspect_format_impl_deserialize
	};

	static struct log_aspect_interface_type aspect_interface_format =
	{
		&log_aspect_format_create,
		&log_aspect_format_impl_obj,
		&log_aspect_format_destroy
	};

	return &aspect_interface_format;
}

static log_aspect_data log_aspect_format_create(log_aspect aspect, const log_aspect_ctor ctor)
{
	/* TODO */
	(void)aspect;
	(void)ctor;

	return NULL;
}

static int log_aspect_format_impl_size_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data)
{
	struct log_aspect_format_size_type * size_args = notify_data;

	log_policy_format_impl format_impl = log_policy_derived(policy);

	size_args->size = format_impl->size(policy, size_args->record);

	(void)aspect;

	if (size_args->size == 0)
	{
		return 1;
	}

	return 0;
}

static size_t log_aspect_format_impl_size(log_aspect aspect, log_record record)
{
	struct log_aspect_format_size_type notify_data;

	notify_data.record = record;
	notify_data.size = 0;

	/* TODO: design error (critical), repleace this in the future */
	if (log_aspect_notify_first(aspect, &log_aspect_format_impl_size_cb, (log_aspect_notify_data)&notify_data) != 0)
	{
		return 0;
	}

	return notify_data.size;
}

static int log_aspect_format_impl_serialize_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data)
{
	struct log_aspect_format_serialize_type * serialize_args = notify_data;

	log_policy_format_impl format_impl = log_policy_derived(policy);

	size_t written_size = format_impl->serialize(policy, serialize_args->record, serialize_args->buffer, serialize_args->size);

	(void)aspect;

	return (written_size != serialize_args->size);
}

static int log_aspect_format_impl_serialize(log_aspect aspect, log_record record, void * buffer, const size_t size)
{
	struct log_aspect_format_serialize_type notify_data;

	notify_data.record = record;
	notify_data.buffer = buffer;
	notify_data.size = size;

	return log_aspect_notify_all(aspect, &log_aspect_format_impl_serialize_cb, (log_aspect_notify_data)&notify_data);
}

static int log_aspect_format_impl_deserialize_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data)
{
	struct log_aspect_format_deserialize_type * deserialize_args = notify_data;

	log_policy_format_impl format_impl = log_policy_derived(policy);

	size_t size = format_impl->deserialize(policy, deserialize_args->record, deserialize_args->buffer, deserialize_args->size);

	(void)aspect;

	return (deserialize_args->size != size);
}

static int log_aspect_format_impl_deserialize(log_aspect aspect, log_record record, const void * buffer, const size_t size)
{
	struct log_aspect_format_deserialize_type notify_data;

	notify_data.record = record;
	notify_data.buffer = buffer;
	notify_data.size = size;

	return log_aspect_notify_all(aspect, &log_aspect_format_impl_deserialize_cb, (log_aspect_notify_data)&notify_data);
}

static int log_aspect_format_destroy(log_aspect aspect)
{
	/* TODO */
	(void)aspect;

	return 0;
}
