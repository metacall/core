/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_POLICY_STREAM_H
#define LOG_POLICY_STREAM_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_policy.h>
#include <log/log_policy_stream_custom.h>
#include <log/log_policy_stream_file.h>
#include <log/log_policy_stream_nginx.h>
#include <log/log_policy_stream_socket.h>
#include <log/log_policy_stream_stdio.h>
#include <log/log_policy_stream_syslog.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

enum log_policy_stream_id
{
	LOG_POLICY_STREAM_FILE = 0x00,
	LOG_POLICY_STREAM_NGINX = 0x01,
	LOG_POLICY_STREAM_SOCKET = 0x02,
	LOG_POLICY_STREAM_STDIO = 0x03,
	LOG_POLICY_STREAM_SYSLOG = 0x04,
	LOG_POLICY_STREAM_CUSTOM = 0x05,

	LOG_POLICY_STREAM_SIZE
};

/* -- Forward Declarations -- */

struct log_policy_stream_impl_type;

/* -- Type Definitions -- */

typedef struct log_policy_stream_impl_type *log_policy_stream_impl;

typedef int (*log_policy_stream_write)(log_policy, const void *, const size_t);
typedef int (*log_policy_stream_flush)(log_policy);

/* -- Member Data -- */

struct log_policy_stream_impl_type
{
	log_policy_stream_write write;
	log_policy_stream_flush flush;
};

/* -- Methods -- */

LOG_API log_policy_interface log_policy_stream(const log_policy_id policy_stream_id);

LOG_API log_policy log_policy_stream_file(const char *file_name, const char *mode);

LOG_API log_policy log_policy_stream_nginx(ngx_log_t *ngx_log_ptr, void (*ngx_error_ptr)(void), uint16_t ngx_log_level);

LOG_API log_policy log_policy_stream_socket(const char *ip, uint16_t port);

LOG_API log_policy log_policy_stream_stdio(FILE *stream);

LOG_API log_policy log_policy_stream_syslog(const char *name);

LOG_API log_policy log_policy_stream_custom(void *context, int (*stream_write)(void *, const char *, const size_t), int (*stream_flush)(void *));

#ifdef __cplusplus
}
#endif

#endif /* LOG_POLICY_STREAM_H */
