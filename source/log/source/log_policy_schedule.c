/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_policy_schedule.h>
#include <log/log_policy_schedule_async.h>
#include <log/log_policy_schedule_sync.h>

/* -- Methods -- */

log_policy_interface log_policy_schedule(const log_policy_id policy_schedule_id)
{
	static const log_policy_singleton policy_schedule_singleton[LOG_POLICY_SCHEDULE_SIZE] = {
		&log_policy_schedule_async_interface,
		&log_policy_schedule_sync_interface
	};

	return policy_schedule_singleton[policy_schedule_id]();
}

log_policy log_policy_schedule_async(void)
{
	return log_policy_create(LOG_ASPECT_SCHEDULE, log_policy_schedule(LOG_POLICY_SCHEDULE_ASYNC), NULL);
}

log_policy log_policy_schedule_sync(void)
{
	return log_policy_create(LOG_ASPECT_SCHEDULE, log_policy_schedule(LOG_POLICY_SCHEDULE_SYNC), NULL);
}
