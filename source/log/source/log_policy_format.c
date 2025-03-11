/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_aspect_format.h>
#include <log/log_policy_format.h>
#include <log/log_policy_format_binary.h>
#include <log/log_policy_format_custom.h>
#include <log/log_policy_format_text.h>

/* -- Methods -- */

log_policy_interface log_policy_format(const log_policy_id policy_format_id)
{
	static const log_policy_singleton policy_format_singleton[LOG_POLICY_FORMAT_SIZE] = {
		&log_policy_format_binary_interface,
		&log_policy_format_text_interface,
		&log_policy_format_custom_interface
	};

	return policy_format_singleton[policy_format_id]();
}

log_policy log_policy_format_binary(void)
{
	return log_policy_create(LOG_ASPECT_FORMAT, log_policy_format(LOG_POLICY_FORMAT_BINARY), NULL);
}

log_policy log_policy_format_text(void)
{
	struct log_policy_format_text_ctor_type text_ctor;

	text_ctor.flags = LOG_POLICY_FORMAT_TEXT_NEWLINE;

	return log_policy_create(LOG_ASPECT_FORMAT, log_policy_format(LOG_POLICY_FORMAT_TEXT), &text_ctor);
}

log_policy log_policy_format_text_flags(unsigned int flags)
{
	struct log_policy_format_text_ctor_type text_ctor;

	text_ctor.flags = flags;

	return log_policy_create(LOG_ASPECT_FORMAT, log_policy_format(LOG_POLICY_FORMAT_TEXT), &text_ctor);
}

log_policy log_policy_format_custom(void *context, log_policy_format_custom_size_ptr format_size, log_policy_format_custom_serialize_ptr format_serialize, log_policy_format_custom_deserialize_ptr format_deserialize)
{
	struct log_policy_format_custom_ctor_type custom_ctor;

	custom_ctor.context = context;
	custom_ctor.format_size = format_size;
	custom_ctor.format_serialize = format_serialize;
	custom_ctor.format_deserialize = format_deserialize;

	return log_policy_create(LOG_ASPECT_FORMAT, log_policy_format(LOG_POLICY_FORMAT_CUSTOM), &custom_ctor);
}
