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
#include <log/log_policy.h>
#include <log/log_impl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdarg.h>

/* -- Methods -- */

LOG_API int log_create(const char * name, log_policy policy, log_impl impl);

LOG_API int log_write(const char * name, const char * tag, enum log_level_id level, const char * message);

LOG_API int log_write_v(const char * name, const char * tag, enum log_level_id level, const char * message, void * args[]);

LOG_API int log_write_va(const char * name, const char * tag, enum log_level_id level, const char * message, ...);

LOG_API int log_clear(const char * name);

LOG_API void log_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_H */
