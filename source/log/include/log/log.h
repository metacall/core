/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_H
#define LOG_H 1

#include <log/log_api.h>

#include <log/log_level.h>
#include <log/log_handler.h>
#include <log/log_policy.h>
#include <log/log_policy_sync.h>
#include <log/log_policy_async.h>
#include <log/log_interface.h>
#include <log/log_impl.h>
#include <log/log_impl_stdio.h>
#include <log/log_impl_file.h>

#ifdef __cplusplus
extern "C" {
#endif

/*LOG_API int log_register(log l);*/

/*LOG_API int log_unregister(log l);*/

LOG_API void log_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_H */
