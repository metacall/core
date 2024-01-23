/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
	LOG_POLICY_SCHEDULE_ASYNC = 0x00,
	LOG_POLICY_SCHEDULE_SYNC = 0x01,

	LOG_POLICY_SCHEDULE_SIZE
};

/* -- Forward Declarations -- */

struct log_policy_schedule_impl_type;

/* -- Type Definitions -- */

typedef void *log_policy_schedule_data;

typedef struct log_policy_schedule_impl_type *log_policy_schedule_impl;

typedef int (*log_policy_schedule_execute_cb)(log_policy, log_policy_schedule_data);

typedef int (*log_policy_schedule_lock)(log_policy);
typedef int (*log_policy_schedule_execute)(log_policy, log_policy_schedule_execute_cb, log_policy_schedule_data);
typedef int (*log_policy_schedule_unlock)(log_policy);

/* -- Member Data -- */

struct log_policy_schedule_impl_type
{
	log_policy_schedule_lock lock;
	log_policy_schedule_execute execute;
	log_policy_schedule_unlock unlock;
};

/* -- Methods -- */

LOG_API log_policy_interface log_policy_schedule(const log_policy_id policy_schedule_id);

LOG_API log_policy log_policy_schedule_async(void);

LOG_API log_policy log_policy_schedule_sync(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_POLICY_SCHEDULE_H */
