/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_policy_storage.h>
#include <log/log_policy_storage_sequential.h>
#include <log/log_policy_storage_batch.h>

/* -- Methods -- */

const log_policy_interface log_policy_storage(enum log_policy_storage_id policy_storage_id)
{
	static const log_policy_singleton policy_storage_singleton[LOG_POLICY_STORAGE_SIZE] =
	{
		&log_policy_storage_sequential,
		&log_policy_storage_batch
	};

	return policy_storage_singleton[policy_storage_id]();
}
