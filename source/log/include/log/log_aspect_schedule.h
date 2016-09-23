/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_ASPECT_SCHEDULE_H
#define LOG_ASPECT_SCHEDULE_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_aspect.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct log_aspect_schedule_impl_type;

/* -- Type Definitions -- */

typedef struct log_aspect_schedule_impl_type * log_aspect_schedule_impl;

typedef int (*log_aspect_schedule_lock)(log_aspect);
typedef int (*log_aspect_schedule_unlock)(log_aspect);

/* -- Member Data -- */

struct log_aspect_schedule_impl_type
{
	log_aspect_schedule_lock lock;
	log_aspect_schedule_unlock unlock;
};

/* -- Methods -- */

LOG_API log_aspect_interface log_aspect_schedule_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_ASPECT_SCHEDULE_H */
