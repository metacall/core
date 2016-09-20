/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_POLICY_STREAM_STDIO_H
#define LOG_POLICY_STREAM_STDIO_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_policy.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdio.h>

/* -- Forward Declarations -- */

struct log_policy_stream_stdio_ctor_type;

/* -- Type Definitions -- */

typedef struct log_policy_stream_stdio_ctor_type * log_policy_stream_stdio_ctor;

/* -- Member Data -- */

struct log_policy_stream_stdio_ctor_type
{
	FILE * stream;
};

/* -- Methods -- */

LOG_API const log_policy_interface log_policy_stream_stdio(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_POLICY_STREAM_STDIO_H */
