/*
*	Logger Library by Parra Studios
*	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A generic logger library providing application execution reports.
*
*/

/* -- Headers -- */

#include <log/log_policy_schedule.h>
#include <log/log_policy_schedule_async.h>

/* -- Forward Declarations -- */

struct log_policy_schedule_async_data_type;

/* -- Type Definitions -- */

typedef struct log_policy_schedule_async_data_type *log_policy_schedule_async_data;

/* -- Member Data -- */

struct log_policy_schedule_async_data_type
{
	/* TODO: this must be implemented with free-lock */
	void *mutex;
};

/* -- Private Methods -- */

static int log_policy_schedule_async_create(log_policy policy, const log_policy_ctor ctor);

static int log_policy_schedule_async_lock(log_policy policy);

static int log_policy_schedule_async_execute(log_policy policy, log_policy_schedule_execute_cb callback, log_policy_schedule_data data);

static int log_policy_schedule_async_unlock(log_policy policy);

static int log_policy_schedule_async_destroy(log_policy policy);

/* -- Methods -- */

log_policy_interface log_policy_schedule_async_interface(void)
{
	static struct log_policy_schedule_impl_type log_policy_schedule_async_impl_obj = {
		&log_policy_schedule_async_lock,
		&log_policy_schedule_async_execute,
		&log_policy_schedule_async_unlock
	};

	static struct log_policy_interface_type policy_interface_schedule = {
		&log_policy_schedule_async_create,
		&log_policy_schedule_async_impl_obj,
		&log_policy_schedule_async_destroy
	};

	return &policy_interface_schedule;
}

static int log_policy_schedule_async_create(log_policy policy, const log_policy_ctor ctor)
{
	log_policy_schedule_async_data async_data = malloc(sizeof(struct log_policy_schedule_async_data_type));

	(void)ctor;

	if (async_data == NULL)
	{
		return 1;
	}

	async_data->mutex = NULL;

	log_policy_instantiate(policy, async_data, LOG_POLICY_SCHEDULE_ASYNC);

	return 0;
}

static int log_policy_schedule_async_lock(log_policy policy)
{
	(void)policy;

	/* mutex_lock(mutex); */

	return 0;
}

static int log_policy_schedule_async_execute(log_policy policy, log_policy_schedule_execute_cb callback, log_policy_schedule_data data)
{
	(void)policy;

	return callback(policy, data);
}

static int log_policy_schedule_async_unlock(log_policy policy)
{
	(void)policy;

	/* mutex_unlock(mutex); */

	return 0;
}

static int log_policy_schedule_async_destroy(log_policy policy)
{
	log_policy_schedule_async_data async_data = log_policy_instance(policy);

	if (async_data != NULL)
	{
		free(async_data);
	}

	return 0;
}
