/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_policy_schedule.h>
#include <log/log_policy_schedule_sync.h>
#include <log/log_policy_schedule_async.h>

/* -- Methods -- */

const log_policy_interface log_policy_schedule(enum log_policy_schedule_id policy_schedule_id)
{
	static log_policy_singleton policy_schedule_singleton[LOG_POLICY_SCHEDULE_SIZE] =
	{
		&log_policy_schedule_sync,
		&log_policy_schedule_async
	};

	return policy_schedule_singleton[policy_schedule_id]();
}
