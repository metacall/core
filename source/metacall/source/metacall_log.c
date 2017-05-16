/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

 /* -- Headers -- */

#include <metacall/metacall-version.h>
#include <metacall/metacall-plugins.h>
#include <metacall/metacall_log.h>

#include <log/log.h>

/* -- Methods -- */

int metacall_log(enum metacall_log_id log_id, void * ctx)
{
	switch (log_id)
	{
		case METACALL_LOG_STDIO :
		{
			metacall_log_stdio stdio_ctx = (metacall_log_stdio)ctx;

			return log_configure("metacall",
				log_policy_format_text(),
				log_policy_schedule_sync(),
				log_policy_storage_sequential(),
				log_policy_stream_stdio(stdio_ctx->stream));
		}

		case METACALL_LOG_FILE :
		{
			metacall_log_file file_ctx = (metacall_log_file)ctx;

			return log_configure("metacall",
				log_policy_format_text(),
				log_policy_schedule_sync(),
				log_policy_storage_sequential(),
				log_policy_stream_file(file_ctx->file_name, file_ctx->mode));
		}

		case METACALL_LOG_SOCKET :
		{
			metacall_log_socket socket_ctx = (metacall_log_socket)ctx;

			return log_configure("metacall",
				log_policy_format_text(),
				log_policy_schedule_sync(),
				log_policy_storage_sequential(),
				log_policy_stream_socket(socket_ctx->ip, socket_ctx->port));
		}

		case METACALL_LOG_SYSLOG :
		{
			metacall_log_syslog syslog_ctx = (metacall_log_syslog)ctx;

			return log_configure("metacall",
				log_policy_format_text(),
				log_policy_schedule_sync(),
				log_policy_storage_sequential(),
				log_policy_stream_syslog(syslog_ctx->name));
		}

		case METACALL_LOG_NGINX :
		{
			metacall_log_nginx nginx_ctx = (metacall_log_nginx)ctx;

			return log_configure("metacall",
				log_policy_format_text(),
				log_policy_schedule_sync(),
				log_policy_storage_sequential(),
				log_policy_stream_nginx(nginx_ctx->log, nginx_ctx->log_error, nginx_ctx->log_level));
		}
	}

	return 1;
}
