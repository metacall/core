/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_aspect_storage.h>
#include <log/log_policy_storage.h>

/* -- Member Data -- */

struct log_aspect_storage_append_type
{
	log_record record;
};

/* -- Private Methods -- */

static log_aspect_data log_aspect_storage_create(log_aspect aspect, const log_aspect_ctor ctor);

static int log_aspect_storage_impl_append_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data);

static int log_aspect_storage_impl_append(log_aspect aspect, const log_record record);

static int log_aspect_storage_impl_flush_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data);

static int log_aspect_storage_impl_flush(log_aspect aspect);

static int log_aspect_storage_destroy(log_aspect aspect);

/* -- Methods -- */

log_aspect_interface log_aspect_storage_interface()
{
	static struct log_aspect_storage_impl_type log_aspect_storage_impl_obj =
	{
		&log_aspect_storage_impl_append,
		&log_aspect_storage_impl_flush
	};

	static struct log_aspect_interface_type aspect_interface_storage =
	{
		&log_aspect_storage_create,
		&log_aspect_storage_impl_obj,
		&log_aspect_storage_destroy
	};

	return &aspect_interface_storage;
}

static log_aspect_data log_aspect_storage_create(log_aspect aspect, const log_aspect_ctor ctor)
{
	/* TODO */
	(void)aspect;
	(void)ctor;

	return NULL;
}

static int log_aspect_storage_impl_append_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data)
{
	struct log_aspect_storage_append_type * append_args = notify_data;

	log_policy_storage_impl storage_impl = log_policy_derived(policy);

	(void)aspect;

	return storage_impl->append(policy, append_args->record);
}

static int log_aspect_storage_impl_append(log_aspect aspect, const log_record record)
{
	struct log_aspect_storage_append_type notify_data;

	notify_data.record = record;

	return log_aspect_notify_all(aspect, &log_aspect_storage_impl_append_cb, (log_aspect_notify_data)&notify_data);
}

static int log_aspect_storage_impl_flush_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data)
{
	log_policy_storage_impl storage_impl = log_policy_derived(policy);

	(void)aspect;
	(void)notify_data;

	return storage_impl->flush(policy);
}

static int log_aspect_storage_impl_flush(log_aspect aspect)
{
	return log_aspect_notify_all(aspect, &log_aspect_storage_impl_flush_cb, NULL);
}

static int log_aspect_storage_destroy(log_aspect aspect)
{
	/* TODO */
	(void)aspect;

	return 0;
}
