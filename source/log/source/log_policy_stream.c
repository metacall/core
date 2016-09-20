/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_policy_stream.h>
#include <log/log_policy_stream_stdio.h>
#include <log/log_policy_stream_file.h>
#include <log/log_policy_stream_syslog.h>
#include <log/log_policy_stream_socket.h>

/* -- Methods -- */

const log_policy_interface log_policy_stream(const log_policy_id policy_stream_id)
{
	static const log_policy_singleton policy_stream_singleton[LOG_POLICY_STREAM_SIZE] =
	{
		&log_policy_stream_stdio,
		&log_policy_stream_file,
		&log_policy_stream_syslog,
		&log_policy_stream_socket
	};

	return policy_stream_singleton[policy_stream_id]();
}
