/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_POLICY_STREAM_SOCKET_H
#define LOG_POLICY_STREAM_SOCKET_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_policy.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdint.h>

/* -- Forward Declarations -- */

struct log_policy_stream_socket_ctor_type;

/* -- Type Definitions -- */

typedef struct log_policy_stream_socket_ctor_type *log_policy_stream_socket_ctor;

/* -- Member Data -- */

struct log_policy_stream_socket_ctor_type
{
	const char *ip;
	uint16_t port;
};

/* -- Methods -- */

LOG_API log_policy_interface log_policy_stream_socket_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_POLICY_STREAM_SOCKET_H */
