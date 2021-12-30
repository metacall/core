/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_policy_storage.h>
#include <log/log_policy_storage_sequential.h>

/* -- Private Methods -- */

static int log_policy_storage_sequential_create(log_policy policy, const log_policy_ctor ctor);

static int log_policy_storage_sequential_append(log_policy policy, const log_record record);

static int log_policy_storage_sequential_flush(log_policy policy);

static int log_policy_storage_sequential_destroy(log_policy policy);

/* -- Methods -- */

log_policy_interface log_policy_storage_sequential_interface(void)
{
	static struct log_policy_storage_impl_type log_policy_storage_sequential_impl_obj = {
		&log_policy_storage_sequential_append,
		&log_policy_storage_sequential_flush
	};

	static struct log_policy_interface_type policy_interface_storage = {
		&log_policy_storage_sequential_create,
		&log_policy_storage_sequential_impl_obj,
		&log_policy_storage_sequential_destroy
	};

	return &policy_interface_storage;
}

static int log_policy_storage_sequential_create(log_policy policy, const log_policy_ctor ctor)
{
	(void)ctor;

	log_policy_instantiate(policy, NULL, LOG_POLICY_STORAGE_SEQUENTIAL);

	return 0;
}

static int log_policy_storage_sequential_append(log_policy policy, const log_record record)
{
	(void)policy;
	(void)record;

	/* TODO */

	return 0;
}

static int log_policy_storage_sequential_flush(log_policy policy)
{
	(void)policy;

	/* TODO */

	return 0;
}

static int log_policy_storage_sequential_destroy(log_policy policy)
{
	(void)policy;

	return 0;
}
