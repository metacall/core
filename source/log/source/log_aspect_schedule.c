/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_aspect_schedule.h>
#include <log/log_policy_schedule.h>

/* -- Private Methods -- */

LOG_NO_EXPORT static log_aspect_data log_aspect_schedule_create(log_aspect aspect, const log_aspect_ctor ctor);

LOG_NO_EXPORT static int log_aspect_schedule_impl_lock_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data);

LOG_NO_EXPORT static int log_aspect_schedule_impl_lock(log_aspect aspect);

LOG_NO_EXPORT static int log_aspect_schedule_impl_unlock_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data);

LOG_NO_EXPORT static int log_aspect_schedule_impl_unlock(log_aspect aspect);

LOG_NO_EXPORT static int log_aspect_schedule_destroy(log_aspect aspect);

/* -- Methods -- */

log_aspect_interface log_aspect_schedule_interface()
{
	static struct log_aspect_schedule_impl_type log_aspect_schedule_impl =
	{
		&log_aspect_schedule_impl_lock,
		&log_aspect_schedule_impl_unlock
	};

	static struct log_aspect_interface_type aspect_interface_schedule =
	{
		&log_aspect_schedule_create,
		&log_aspect_schedule_impl,
		&log_aspect_schedule_destroy
	};

	return &aspect_interface_schedule;
}

static log_aspect_data log_aspect_schedule_create(log_aspect aspect, const log_aspect_ctor ctor)
{
	/* TODO */
	(void)aspect;
	(void)ctor;

	return NULL;
}

static int log_aspect_schedule_impl_lock_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data)
{
	log_policy_schedule_impl schedule_impl = log_policy_derived(policy);

	(void)aspect;
	(void)notify_data;

	return schedule_impl->lock(policy);
}

static int log_aspect_schedule_impl_lock(log_aspect aspect)
{
	return log_aspect_notify_all(aspect, &log_aspect_schedule_impl_lock_cb, NULL);
}

static int log_aspect_schedule_impl_unlock_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data)
{
	log_policy_schedule_impl schedule_impl = log_policy_derived(policy);

	(void)aspect;
	(void)notify_data;

	return schedule_impl->unlock(policy);
}

static int log_aspect_schedule_impl_unlock(log_aspect aspect)
{
	return log_aspect_notify_all(aspect, &log_aspect_schedule_impl_unlock_cb, NULL);
}

static int log_aspect_schedule_destroy(log_aspect aspect)
{
	/* TODO */
	(void)aspect;

	return 0;
}
