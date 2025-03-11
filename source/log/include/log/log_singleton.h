/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_SINGLETON_H
#define LOG_SINGLETON_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_impl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct log_singleton_map_bucket_type;

struct log_singleton_type;

/* -- Type Definitions -- */

typedef struct log_singleton_map_bucket_type *log_singleton_map_bucket;

typedef struct log_singleton_type *log_singleton;

/* -- Methods -- */

LOG_API log_singleton *log_singleton_instance(void);

LOG_API void log_singleton_initialize(log_singleton singleton);

LOG_API size_t log_singleton_size(void);

LOG_API int log_singleton_insert(const char *name, log_impl impl);

LOG_API log_impl log_singleton_get(const char *name);

LOG_API log_impl log_singleton_remove(const char *name);

LOG_API void log_singleton_clear(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_SINGLETON_H */
