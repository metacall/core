/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_IMPL_H
#define LOG_IMPL_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_record.h>
#include <log/log_aspect.h>
#include <log/log_handle.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct log_impl_type;

/* -- Type Definitions -- */

typedef struct log_impl_type * log_impl;

/* -- Methods -- */

LOG_API log_aspect_interface log_impl_aspect_interface(enum log_aspect_id aspect_id);

LOG_API log_impl log_impl_create(const char * name);

LOG_API void log_impl_define(log_impl impl, log_aspect aspect, enum log_aspect_id aspect_id);

LOG_API const char * log_impl_name(log_impl impl);

LOG_API log_handle log_impl_handle(log_impl impl);

LOG_API log_aspect log_impl_aspect(log_impl impl, enum log_aspect_id aspect_id);

LOG_API int log_impl_write(log_impl impl, const log_record_ctor record_ctor);

LOG_API int log_impl_clear(log_impl impl);

LOG_API int log_impl_destroy(log_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* LOG_IMPL_H */
