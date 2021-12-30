/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_policy_stream.h>
#include <log/log_policy_stream_nginx.h>

#if defined(_WIN32)
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif

	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif

	#include <windows.h>
#endif

/* -- Forward Declarations -- */

#if defined(_WIN32)
typedef DWORD ngx_err_t;
#else
typedef int ngx_err_t;
#endif

struct log_policy_stream_nginx_data_type;

/* -- Type Definitions -- */

typedef void (*log_policy_stream_nginx_error)(uintptr_t, ngx_log_t *, ngx_err_t, const char *, ...);

typedef struct log_policy_stream_nginx_data_type *log_policy_stream_nginx_data;

/* -- Member Data -- */

struct log_policy_stream_nginx_data_type
{
	ngx_log_t *ngx_log_ptr;
	log_policy_stream_nginx_error ngx_error_ptr;
	uint16_t ngx_log_level;
};

/* -- Private Methods -- */

static int log_policy_stream_nginx_create(log_policy policy, const log_policy_ctor ctor);

static int log_policy_stream_nginx_write(log_policy policy, const void *buffer, const size_t size);

static int log_policy_stream_nginx_flush(log_policy policy);

static int log_policy_stream_nginx_destroy(log_policy policy);

/* -- Methods -- */

log_policy_interface log_policy_stream_nginx_interface(void)
{
	static struct log_policy_stream_impl_type log_policy_stream_nginx_impl_obj = {
		&log_policy_stream_nginx_write,
		&log_policy_stream_nginx_flush
	};

	static struct log_policy_interface_type policy_interface_stream = {
		&log_policy_stream_nginx_create,
		&log_policy_stream_nginx_impl_obj,
		&log_policy_stream_nginx_destroy
	};

	return &policy_interface_stream;
}

static int log_policy_stream_nginx_create(log_policy policy, const log_policy_ctor ctor)
{
	log_policy_stream_nginx_data nginx_data = NULL;

	const log_policy_stream_nginx_ctor nginx_ctor = ctor;

	if (nginx_ctor->ngx_log_ptr == NULL || nginx_ctor->ngx_error_ptr == NULL)
	{
		return 1;
	}

	nginx_data = malloc(sizeof(struct log_policy_stream_nginx_data_type));

	if (nginx_data == NULL)
	{
		return 1;
	}

	nginx_data->ngx_log_ptr = nginx_ctor->ngx_log_ptr;

	nginx_data->ngx_error_ptr = (log_policy_stream_nginx_error)nginx_ctor->ngx_error_ptr;

	nginx_data->ngx_log_level = nginx_ctor->ngx_log_level;

	log_policy_instantiate(policy, nginx_data, LOG_POLICY_STREAM_NGINX);

	return 0;
}

static int log_policy_stream_nginx_write(log_policy policy, const void *buffer, const size_t size)
{
	log_policy_stream_nginx_data nginx_data = log_policy_instance(policy);

	(void)size;

	nginx_data->ngx_error_ptr(nginx_data->ngx_log_level, nginx_data->ngx_log_ptr, 0, (const char *)buffer);

	return 0;
}

static int log_policy_stream_nginx_flush(log_policy policy)
{
	(void)policy;

	return 0;
}

static int log_policy_stream_nginx_destroy(log_policy policy)
{
	log_policy_stream_nginx_data nginx_data = log_policy_instance(policy);

	if (nginx_data != NULL)
	{
		free(nginx_data);
	}

	return 0;
}
