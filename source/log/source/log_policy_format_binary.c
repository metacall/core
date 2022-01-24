/*
*	Logger Library by Parra Studios
*	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A generic logger library providing application execution reports.
*
*/

/* -- Headers -- */

#include <log/log_policy_format.h>
#include <log/log_policy_format_binary.h>

/* -- Forward Declarations -- */

struct log_policy_format_binary_data_type;

/* -- Type Definitions -- */

typedef struct log_policy_format_binary_data_type *log_policy_format_binary_data;

/* -- Member Data -- */

struct log_policy_format_binary_data_type
{
	void *todo;
};

/* -- Private Methods -- */

static int log_policy_format_binary_create(log_policy policy, const log_policy_ctor ctor);

static size_t log_policy_format_binary_size(log_policy policy, const log_record record);

static size_t log_policy_format_binary_serialize(log_policy policy, const log_record record, void *buffer, const size_t size);

static size_t log_policy_format_binary_deserialize(log_policy policy, log_record record, const void *buffer, const size_t size);

static int log_policy_format_binary_destroy(log_policy policy);

/* -- Methods -- */

log_policy_interface log_policy_format_binary_interface(void)
{
	static struct log_policy_format_impl_type log_policy_format_binary_impl_obj = {
		&log_policy_format_binary_size,
		&log_policy_format_binary_serialize,
		&log_policy_format_binary_deserialize
	};

	static struct log_policy_interface_type policy_interface_format = {
		&log_policy_format_binary_create,
		&log_policy_format_binary_impl_obj,
		&log_policy_format_binary_destroy
	};

	return &policy_interface_format;
}

static int log_policy_format_binary_create(log_policy policy, const log_policy_ctor ctor)
{
	log_policy_format_binary_data binary_data = malloc(sizeof(struct log_policy_format_binary_data_type));

	(void)ctor;

	if (binary_data == NULL)
	{
		return 1;
	}

	log_policy_instantiate(policy, binary_data, LOG_POLICY_FORMAT_BINARY);

	return 0;
}

static size_t log_policy_format_binary_size(log_policy policy, const log_record record)
{
	const size_t mock_binary_size = 1;

	(void)policy;
	(void)record;

	/* TODO: implement properly this */
	return mock_binary_size;
}

static size_t log_policy_format_binary_serialize(log_policy policy, const log_record record, void *buffer, const size_t size)
{
	log_policy_format_binary_data binary_data = log_policy_instance(policy);

	(void)binary_data;
	(void)record;
	(void)size;

	/* TODO: implement properly this */
	*((char *)buffer) = '\0';

	return size;
}

static size_t log_policy_format_binary_deserialize(log_policy policy, log_record record, const void *buffer, const size_t size)
{
	log_policy_format_binary_data binary_data = log_policy_instance(policy);

	(void)binary_data;
	(void)record;
	(void)buffer;
	(void)size;

	return size;
}

static int log_policy_format_binary_destroy(log_policy policy)
{
	log_policy_format_binary_data binary_data = log_policy_instance(policy);

	if (binary_data != NULL)
	{
		free(binary_data);
	}

	return 0;
}
