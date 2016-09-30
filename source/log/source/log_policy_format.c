/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_policy_format.h>
#include <log/log_aspect_format.h>
#include <log/log_policy_format_binary.h>
#include <log/log_policy_format_text.h>


/* -- Methods -- */

log_policy_interface log_policy_format(const log_policy_id policy_format_id)
{
	static const log_policy_singleton policy_format_singleton[LOG_POLICY_FORMAT_SIZE] =
	{
		&log_policy_format_binary_interface,
		&log_policy_format_text_interface
	};

	return policy_format_singleton[policy_format_id]();
}

log_policy log_policy_format_binary()
{
	return log_policy_create(LOG_ASPECT_FORMAT, log_policy_format(LOG_POLICY_FORMAT_BINARY), NULL);
}

log_policy log_policy_format_text()
{
	return log_policy_create(LOG_ASPECT_FORMAT, log_policy_format(LOG_POLICY_FORMAT_TEXT), NULL);
}
