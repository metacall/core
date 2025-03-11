/*
*	Logger Library by Parra Studios
*	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A generic logger library providing application execution reports.
*
*/

/* -- Headers -- */

#include <log/log_policy_format.h>
#include <log/log_policy_format_custom.h>

/* -- Forward Declarations -- */

struct log_policy_format_custom_data_type;

/* -- Type Definitions -- */

typedef struct log_policy_format_custom_data_type *log_policy_format_custom_data;

/* -- Member Data -- */

struct log_policy_format_custom_data_type
{
	void *context;
	log_policy_format_custom_size_ptr format_size;
	log_policy_format_custom_serialize_ptr format_serialize;
	log_policy_format_custom_deserialize_ptr format_deserialize;
};

/* -- Private Methods -- */

static int log_policy_format_custom_create(log_policy policy, const log_policy_ctor ctor);

static size_t log_policy_format_custom_size(log_policy policy, const log_record record);

static size_t log_policy_format_custom_serialize(log_policy policy, const log_record record, void *buffer, const size_t size);

static size_t log_policy_format_custom_deserialize(log_policy policy, log_record record, const void *buffer, const size_t size);

static int log_policy_format_custom_destroy(log_policy policy);

/* -- Methods -- */

log_policy_interface log_policy_format_custom_interface(void)
{
	static struct log_policy_format_impl_type log_policy_format_custom_impl_obj = {
		&log_policy_format_custom_size,
		&log_policy_format_custom_serialize,
		&log_policy_format_custom_deserialize
	};

	static struct log_policy_interface_type policy_interface_format = {
		&log_policy_format_custom_create,
		&log_policy_format_custom_impl_obj,
		&log_policy_format_custom_destroy
	};

	return &policy_interface_format;
}

static int log_policy_format_custom_create(log_policy policy, const log_policy_ctor ctor)
{
	log_policy_format_custom_data custom_data = NULL;

	const log_policy_format_custom_ctor custom_ctor = ctor;

	if (custom_ctor->format_size == NULL || custom_ctor->format_serialize == NULL || custom_ctor->format_deserialize == NULL)
	{
		return 1;
	}

	custom_data = malloc(sizeof(struct log_policy_format_custom_data_type));

	if (custom_data == NULL)
	{
		return 1;
	}

	custom_data->format_size = custom_ctor->format_size;
	custom_data->format_serialize = custom_ctor->format_serialize;
	custom_data->format_deserialize = custom_ctor->format_deserialize;
	custom_data->context = custom_ctor->context;

	log_policy_instantiate(policy, custom_data, LOG_POLICY_FORMAT_CUSTOM);

	return 0;
}

static size_t log_policy_format_custom_size(log_policy policy, const log_record record)
{
	log_policy_format_custom_data custom_data = log_policy_instance(policy);

	return custom_data->format_size(custom_data->context,
		ctime(log_record_time(record)),
		log_record_thread_id(record),
		log_record_line(record),
		log_record_func(record),
		log_record_file(record),
		log_level_to_string(log_record_level(record)),
		log_record_message(record),
		(log_policy_format_custom_va_list)log_record_variable_args(record));
}

static size_t log_policy_format_custom_serialize(log_policy policy, const log_record record, void *buffer, const size_t size)
{
	log_policy_format_custom_data custom_data = log_policy_instance(policy);

	return custom_data->format_serialize(custom_data->context, buffer, size,
		ctime(log_record_time(record)),
		log_record_thread_id(record),
		log_record_line(record),
		log_record_func(record),
		log_record_file(record),
		log_level_to_string(log_record_level(record)),
		log_record_message(record),
		(log_policy_format_custom_va_list)log_record_variable_args(record));
}

static size_t log_policy_format_custom_deserialize(log_policy policy, log_record record, const void *buffer, const size_t size)
{
	log_policy_format_custom_data custom_data = log_policy_instance(policy);

	return custom_data->format_deserialize(custom_data->context, buffer, size,
		ctime(log_record_time(record)),
		log_record_thread_id(record),
		log_record_line(record),
		log_record_func(record),
		log_record_file(record),
		log_level_to_string(log_record_level(record)),
		log_record_message(record),
		(log_policy_format_custom_va_list)log_record_variable_args(record));
}

static int log_policy_format_custom_destroy(log_policy policy)
{
	log_policy_format_custom_data custom_data = log_policy_instance(policy);

	if (custom_data != NULL)
	{
		free(custom_data);
	}

	return 0;
}
