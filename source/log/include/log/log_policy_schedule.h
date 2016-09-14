/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_POLICY_SCHEDULE_H
#define LOG_POLICY_SCHEDULE_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_policy.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

enum log_policy_schedule_id
{
	LOG_POLICY_SCHEDULE_SYNC	= 0x00,
	LOG_POLICY_SCHEDULE_ASYNC	= 0x01,

	LOG_POLICY_SCHEDULE_SIZE
};

/* -- Type Definitions -- */

typedef int (*log_policy_schedule_lock)(log_policy);
typedef int (*log_policy_schedule_unlock)(log_policy);

/* -- Member Data -- */

struct log_policy_schedule_impl_type
{
	log_policy_schedule_lock lock;
	log_policy_schedule_unlock unlock;
};

/* -- Methods -- */

LOG_API const log_policy_interface log_policy_schedule(enum log_policy_schedule_id policy_schedule_id);

#ifdef __cplusplus
}
#endif

#endif /* LOG_POLICY_SCHEDULE_H */
