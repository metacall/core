/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_POLICY_STREAM_NGINX_H
#define LOG_POLICY_STREAM_NGINX_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_policy.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdint.h>

/* -- Forward Declarations -- */

struct ngx_log_s;

struct log_policy_stream_nginx_ctor_type;

/* -- Type Definitions -- */

typedef struct ngx_log_s ngx_log_t;

typedef struct log_policy_stream_nginx_ctor_type *log_policy_stream_nginx_ctor;

/* -- Member Data -- */

struct log_policy_stream_nginx_ctor_type
{
	ngx_log_t *ngx_log_ptr;
	void (*ngx_error_ptr)(void);
	uint16_t ngx_log_level;
};

/* -- Methods -- */

LOG_API log_policy_interface log_policy_stream_nginx_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_POLICY_STREAM_NGINX_H */
