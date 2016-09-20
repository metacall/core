/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_POLICY_STREAM_H
#define LOG_POLICY_STREAM_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_policy.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

enum log_policy_stream_id
{
	LOG_POLICY_STREAM_STDIO		= 0x00,
	LOG_POLICY_STREAM_FILE		= 0x01,
	LOG_POLICY_STREAM_SYSLOG	= 0x02,
	LOG_POLICY_STREAM_SOCKET	= 0x03,

	LOG_POLICY_STREAM_SIZE
};

/* -- Type Definitions -- */

typedef int (*log_policy_stream_write)(log_policy, const void * buffer, const size_t size);
typedef int (*log_policy_stream_flush)(log_policy);

/* -- Member Data -- */

struct log_policy_stream_impl_type
{
	log_policy_stream_write write;
	log_policy_stream_flush flush;
};

/* -- Methods -- */

LOG_API const log_policy_interface log_policy_stream(enum log_policy_stream_id policy_stream_id);

#ifdef __cplusplus
}
#endif

#endif /* LOG_POLICY_STREAM_H */
