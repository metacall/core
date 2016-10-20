/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_H
#define LOG_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_level.h>
#include <log/log_record.h>
#include <log/log_policy.h>
#include <log/log_policy_format.h>
#include <log/log_policy_schedule.h>
#include <log/log_policy_storage.h>
#include <log/log_policy_stream.h>
#include <log/log_preprocessor.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdarg.h>

/* -- Methods -- */

LOG_API int log_create(const char * name);

LOG_API int log_define(const char * name, log_policy policy);

LOG_API int log_configure_impl(const char * name, size_t size, ...);

LOG_API int log_write_impl(const char * name, const size_t line, const char * func, const char * file, const enum log_level_id level, const char * message);

LOG_API int log_write_impl_va(const char * name, const size_t line, const char * func, const char * file, const enum log_level_id level, const char * message, ...);

LOG_API int log_clear(const char * name);

LOG_API int log_delete(const char * name);

LOG_API const char * log_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_H */
