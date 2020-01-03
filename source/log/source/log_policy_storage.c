/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_policy_storage.h>
#include <log/log_policy_storage_sequential.h>
#include <log/log_policy_storage_batch.h>

/* -- Methods -- */

log_policy_interface log_policy_storage(const log_policy_id policy_storage_id)
{
	static const log_policy_singleton policy_storage_singleton[LOG_POLICY_STORAGE_SIZE] =
	{
		&log_policy_storage_batch_interface,
		&log_policy_storage_sequential_interface
	};

	return policy_storage_singleton[policy_storage_id]();
}

log_policy log_policy_storage_batch(size_t size)
{
	struct log_policy_storage_batch_ctor_type batch_ctor;

	batch_ctor.size = size;

	return log_policy_create(LOG_ASPECT_STORAGE, log_policy_storage(LOG_POLICY_STORAGE_BATCH), &batch_ctor);
}

log_policy log_policy_storage_sequential(void)
{
	return log_policy_create(LOG_ASPECT_STORAGE, log_policy_storage(LOG_POLICY_STORAGE_SEQUENTIAL), NULL);
}
