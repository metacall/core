/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_THREAD_ID_H
#define LOG_THREAD_ID_H 1

/* -- Headers -- */

#include <log/log_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Methods -- */

size_t log_thread_id(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_THREAD_ID_H */
