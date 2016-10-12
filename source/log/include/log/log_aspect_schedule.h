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

typedef void * log_aspect_schedule_data;

typedef struct log_aspect_schedule_impl_type * log_aspect_schedule_impl;

typedef int (*log_aspect_schedule_execute_cb)(log_policy, log_aspect_schedule_data);

typedef int (*log_aspect_schedule_execute)(log_aspect, log_aspect_schedule_execute_cb, log_aspect_schedule_data);

/* -- Member Data -- */

struct log_aspect_schedule_impl_type
{
	log_aspect_schedule_execute execute;
};

/* -- Methods -- */

LOG_API log_aspect_interface log_aspect_schedule_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_ASPECT_SCHEDULE_H */
