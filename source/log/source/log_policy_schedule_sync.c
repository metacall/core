/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_policy_schedule_sync.h>
#include <log/log_policy_schedule.h>

/* -- Private Methods -- */

LOG_NO_EXPORT static int log_policy_schedule_sync_create(log_policy policy, const log_policy_ctor ctor);

LOG_NO_EXPORT static int log_policy_schedule_sync_lock(log_policy policy);

LOG_NO_EXPORT static int log_policy_schedule_sync_unlock(log_policy policy);

LOG_NO_EXPORT static int log_policy_schedule_sync_destroy(log_policy policy);

/* -- Methods -- */

log_policy_interface log_policy_schedule_sync()
{
	static struct log_policy_schedule_impl_type log_policy_schedule_sync_impl =
	{
		&log_policy_schedule_sync_lock,
		&log_policy_schedule_sync_unlock
	};

	static struct log_policy_interface_type policy_interface_schedule =
	{
		&log_policy_schedule_sync_create,
		&log_policy_schedule_sync_impl,
		&log_policy_schedule_sync_destroy
	};

	return &policy_interface_schedule;
}

static int log_policy_schedule_sync_create(log_policy policy, const log_policy_ctor ctor)
{
	(void)policy;
	(void)ctor;

	return 0;
}

static int log_policy_schedule_sync_lock(log_policy policy)
{
	(void)policy;

	return 0;
}

static int log_policy_schedule_sync_unlock(log_policy policy)
{
	(void)policy;

	return 0;
}

static int log_policy_schedule_sync_destroy(log_policy policy)
{
	(void)policy;

	return 0;
}
