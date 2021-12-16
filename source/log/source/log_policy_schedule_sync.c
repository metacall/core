/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_policy_schedule.h>
#include <log/log_policy_schedule_sync.h>

/* -- Private Methods -- */

static int log_policy_schedule_sync_create(log_policy policy, const log_policy_ctor ctor);

static int log_policy_schedule_sync_lock(log_policy policy);

static int log_policy_schedule_sync_execute(log_policy policy, log_policy_schedule_execute_cb cb, log_policy_schedule_data data);

static int log_policy_schedule_sync_unlock(log_policy policy);

static int log_policy_schedule_sync_destroy(log_policy policy);

/* -- Methods -- */

log_policy_interface log_policy_schedule_sync_interface(void)
{
	static struct log_policy_schedule_impl_type log_policy_schedule_sync_impl_obj = {
		&log_policy_schedule_sync_lock,
		&log_policy_schedule_sync_execute,
		&log_policy_schedule_sync_unlock
	};

	static struct log_policy_interface_type policy_interface_schedule = {
		&log_policy_schedule_sync_create,
		&log_policy_schedule_sync_impl_obj,
		&log_policy_schedule_sync_destroy
	};

	return &policy_interface_schedule;
}

static int log_policy_schedule_sync_create(log_policy policy, const log_policy_ctor ctor)
{
	(void)ctor;

	log_policy_instantiate(policy, NULL, LOG_POLICY_SCHEDULE_SYNC);

	return 0;
}

static int log_policy_schedule_sync_lock(log_policy policy)
{
	(void)policy;

	return 0;
}

static int log_policy_schedule_sync_execute(log_policy policy, log_policy_schedule_execute_cb callback, log_policy_schedule_data data)
{
	(void)policy;

	return callback(policy, data);
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
