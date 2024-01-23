/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_POLICY_SCHEDULE_ASYNC_H
#define LOG_POLICY_SCHEDULE_ASYNC_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_policy.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

LOG_API log_policy_interface log_policy_schedule_async_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_POLICY_SCHEDULE_ASYNC_H */
