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
#include <log/log_policy_schedule.h>
#include <log/log_policy_storage.h>
#include <log/log_policy_stream.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdarg.h>

/* -- Macros -- */

#define log_write(name, tag, level, message) \
	log_write_impl(name, tag, __LINE__, log_record_function(), __FILE__, level, message)

#define log_write_v(name, tag, level, message, args) \
	log_write_impl_v(name, tag, __LINE__, log_record_function(), __FILE__, level, message, args)

#define log_write_va(name, tag, level, message, ...) \
	log_write_impl(name, tag, __LINE__, log_record_function(), __FILE__, level, message, __VA_ARGS__)

/* -- Methods -- */

LOG_API int log_create(const char * name);

LOG_API int log_define(const char * name, log_policy policy);

LOG_API int log_write_impl(const char * name, const char * tag, const size_t line, const char * func, const char * file, const enum log_level_id level, const char * message);

LOG_API int log_write_impl_v(const char * name, const char * tag, const size_t line, const char * func, const char * file, const enum log_level_id level, const char * message, void * args[]);

LOG_API int log_write_impl_va(const char * name, const char * tag, const size_t line, const char * func, const char * file, const enum log_level_id level, const char * message, ...);

LOG_API int log_clear(const char * name);

LOG_API int log_delete(const char * name);

LOG_API void log_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_H */
