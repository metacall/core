/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_ASPECT_STREAM_H
#define LOG_ASPECT_STREAM_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_aspect.h>
#include <log/log_record.h>
#include <log/log_handle.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct log_aspect_stream_impl_type;

/* -- Type Definitions -- */

typedef struct log_aspect_stream_impl_type * log_aspect_stream_impl;

typedef int (*log_aspect_stream_write)(log_aspect, const log_record_ctor);
typedef int (*log_aspect_stream_flush)(log_aspect);

/* -- Member Data -- */

struct log_aspect_stream_impl_type
{
	log_aspect_stream_write write;
	log_aspect_stream_flush flush;
};

/* -- Methods -- */

LOG_API log_aspect_interface log_aspect_stream_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_ASPECT_STREAM_H */
