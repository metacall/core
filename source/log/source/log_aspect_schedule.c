/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_aspect_schedule.h>
#include <log/log_policy_schedule.h>

/* -- Forward Declarations -- */

struct log_aspect_schedule_notify_data_type;

/* -- Type Definitions -- */

typedef struct log_aspect_schedule_notify_data_type *log_aspect_schedule_notify_data;

/* -- Member Data -- */

struct log_aspect_schedule_notify_data_type
{
	log_aspect_schedule_execute_cb callback;
	log_aspect_schedule_data data;
};

/* -- Private Methods -- */

static log_aspect_data log_aspect_schedule_create(log_aspect aspect, const log_aspect_ctor ctor);

static int log_aspect_schedule_impl_execute_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data);

static int log_aspect_schedule_impl_execute(log_aspect aspect, log_aspect_schedule_execute_cb callback, log_aspect_schedule_data data);

static int log_aspect_schedule_destroy(log_aspect aspect);

/* -- Methods -- */

log_aspect_interface log_aspect_schedule_interface(void)
{
	static struct log_aspect_schedule_impl_type log_aspect_schedule_impl_obj = {
		&log_aspect_schedule_impl_execute
	};

	static struct log_aspect_interface_type aspect_interface_schedule = {
		&log_aspect_schedule_create,
		&log_aspect_schedule_impl_obj,
		&log_aspect_schedule_destroy
	};

	return &aspect_interface_schedule;
}

static log_aspect_data log_aspect_schedule_create(log_aspect aspect, const log_aspect_ctor ctor)
{
	/* TODO */
	(void)aspect;
	(void)ctor;

	return NULL;
}

static int log_aspect_schedule_impl_execute_cb(log_aspect aspect, log_policy policy, log_aspect_notify_data notify_data)
{
	log_aspect_schedule_notify_data args = notify_data;

	log_policy_schedule_impl schedule_impl = log_policy_derived(policy);

	(void)aspect;

	return schedule_impl->execute(policy, args->callback, args->data);
}

static int log_aspect_schedule_impl_execute(log_aspect aspect, log_aspect_schedule_execute_cb callback, log_aspect_schedule_data data)
{
	struct log_aspect_schedule_notify_data_type notify_data;

	notify_data.callback = callback;
	notify_data.data = data;

	return log_aspect_notify_first(aspect, &log_aspect_schedule_impl_execute_cb, &notify_data);
}

static int log_aspect_schedule_destroy(log_aspect aspect)
{
	/* TODO */
	(void)aspect;

	return 0;
}
