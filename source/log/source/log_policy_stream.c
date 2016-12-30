/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_policy_stream.h>
#include <log/log_aspect_stream.h>
#include <log/log_policy_stream_file.h>
#include <log/log_policy_stream_nginx.h>
#include <log/log_policy_stream_socket.h>
#include <log/log_policy_stream_stdio.h>
#include <log/log_policy_stream_syslog.h>


/* -- Methods -- */

log_policy_interface log_policy_stream(const log_policy_id policy_stream_id)
{
	static const log_policy_singleton policy_stream_singleton[LOG_POLICY_STREAM_SIZE] =
	{
		&log_policy_stream_file_interface,
		&log_policy_stream_nginx_interface,
		&log_policy_stream_socket_interface,
		&log_policy_stream_stdio_interface,
		&log_policy_stream_syslog_interface
	};

	return policy_stream_singleton[policy_stream_id]();
}

log_policy log_policy_stream_file(const char * file_name, const char * mode)
{
	log_policy policy;

	struct log_policy_stream_file_ctor_type file_ctor;

	file_ctor.file_name = file_name;
	file_ctor.mode = mode;

	policy = log_policy_create(LOG_ASPECT_STREAM, log_policy_stream(LOG_POLICY_STREAM_FILE), &file_ctor);

	if (policy == NULL)
	{
		return NULL;
	}

	return policy;
}

log_policy log_policy_stream_nginx(ngx_log_t * ngx_log_ptr, void (*ngx_error_ptr)())
{
	log_policy policy;

	struct log_policy_stream_nginx_ctor_type nginx_ctor;

	nginx_ctor.ngx_log_ptr = ngx_log_ptr;
	nginx_ctor.ngx_error_ptr = ngx_error_ptr;

	policy = log_policy_create(LOG_ASPECT_STREAM, log_policy_stream(LOG_POLICY_STREAM_NGINX), &nginx_ctor);

	if (policy == NULL)
	{
		return NULL;
	}

	return policy;
}

log_policy log_policy_stream_socket(const char * ip, uint16_t port)
{
	log_policy policy;

	struct log_policy_stream_socket_ctor_type socket_ctor;

	socket_ctor.ip = ip;
	socket_ctor.port = port;

	policy = log_policy_create(LOG_ASPECT_STREAM, log_policy_stream(LOG_POLICY_STREAM_SOCKET), &socket_ctor);

	if (policy == NULL)
	{
		return NULL;
	}

	return policy;
}

log_policy log_policy_stream_stdio(FILE * stream)
{
	log_policy policy;

	struct log_policy_stream_stdio_ctor_type stdio_ctor;

	stdio_ctor.stream = stream;

	policy = log_policy_create(LOG_ASPECT_STREAM, log_policy_stream(LOG_POLICY_STREAM_STDIO), &stdio_ctor);

	if (policy == NULL)
	{
		return NULL;
	}

	return policy;
}

log_policy log_policy_stream_syslog(const char * name)
{
	log_policy policy;

	struct log_policy_stream_syslog_ctor_type syslog_ctor;

	syslog_ctor.name = name;

	policy = log_policy_create(LOG_ASPECT_STREAM, log_policy_stream(LOG_POLICY_STREAM_SYSLOG), &syslog_ctor);

	if (policy == NULL)
	{
		return NULL;
	}

	return policy;
}
