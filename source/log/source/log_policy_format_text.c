/*
*	Logger Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A generic logger library providing application execution reports.
*
*/

/* -- Headers -- */

#include <log/log_policy_format_text.h>
#include <log/log_policy_format.h>
#include <log/log_level.h>

#include <format/format_print.h>

#include <time.h>

/* -- Definitions -- */

#define LOG_POLICY_FORMAT_TEXT_STR "[%.19s] #%" PRIuS " [ %" PRIuS " | %s | %s ] @%s : "

/* -- Forward Declarations -- */

struct log_policy_format_text_data_type;

/* -- Type Definitions -- */

typedef struct log_policy_format_text_data_type * log_policy_format_text_data;

/* -- Member Data -- */

struct log_policy_format_text_data_type
{
	void * todo;
};

/* -- Private Methods -- */

static int log_policy_format_text_create(log_policy policy, const log_policy_ctor ctor);

static size_t log_policy_format_text_size(log_policy policy, const log_record record);

static size_t log_policy_format_text_serialize(log_policy policy, const log_record record, void * buffer, const size_t size);

static size_t log_policy_format_text_serialize_impl(log_policy policy, const log_record record, void * buffer, const size_t size);

static size_t log_policy_format_text_serialize_impl_va(log_policy policy, const log_record record, void * buffer, const size_t size);

static size_t log_policy_format_text_deserialize(log_policy policy, log_record record, const void * buffer, const size_t size);

static int log_policy_format_text_destroy(log_policy policy);

/* -- Methods -- */

log_policy_interface log_policy_format_text_interface()
{
	static struct log_policy_format_impl_type log_policy_format_text_impl_obj =
	{
		&log_policy_format_text_size,
		&log_policy_format_text_serialize,
		&log_policy_format_text_deserialize
	};

	static struct log_policy_interface_type policy_interface_format =
	{
		&log_policy_format_text_create,
		&log_policy_format_text_impl_obj,
		&log_policy_format_text_destroy
	};

	return &policy_interface_format;
}

static int log_policy_format_text_create(log_policy policy, const log_policy_ctor ctor)
{
	log_policy_format_text_data text_data = malloc(sizeof(struct log_policy_format_text_data_type));

	(void)ctor;

	if (text_data == NULL)
	{
		return 1;
	}

	log_policy_instantiate(policy, text_data, LOG_POLICY_FORMAT_TEXT);

	return 0;
}

static size_t log_policy_format_text_size(log_policy policy, const log_record record)
{
	return log_policy_format_text_serialize(policy, record, NULL, 0);
}

static size_t log_policy_format_text_serialize_impl(log_policy policy, const log_record record, void * buffer, const size_t size)
{
	static const char format[] = LOG_POLICY_FORMAT_TEXT_STR "%s\n";

	log_policy_format_text_data text_data = log_policy_instance(policy);

	int result;

	(void)text_data;

	result = snprintf(buffer, size, format,
		ctime(log_record_time(record)),
		log_record_thread_id(record),
		log_record_line(record),
		log_record_func(record),
		log_record_file(record),
		log_level_name(log_record_level(record)),
		log_record_message(record));

	if (result <= 0)
	{
		return 0;
	}

	if (buffer != NULL)
	{
		*(((char *)buffer) + result) = '\0';
	}

	return (size_t)result;
}

static size_t log_policy_format_text_serialize_impl_va(log_policy policy, const log_record record, void * buffer, const size_t size)
{
	static const char header_format[] = LOG_POLICY_FORMAT_TEXT_STR;

	log_policy_format_text_data text_data = log_policy_instance(policy);

	int header_size = 0, body_size = 0;

	void * buffer_body = NULL;

	(void)text_data;

	header_size = snprintf(buffer, size, header_format,
		ctime(log_record_time(record)),
		log_record_thread_id(record),
		log_record_line(record),
		log_record_func(record),
		log_record_file(record),
		log_level_name(log_record_level(record)));

	if (header_size <= 0)
	{
		return 0;
	}

	if (buffer != NULL)
	{
		buffer_body = (void *)(((char *)buffer) + header_size);
	}

	if (log_record_data(record) != NULL)
	{
		va_list args_copy;

		va_copy(args_copy, log_record_data(record));

		body_size = vsnprintf(buffer_body, size, log_record_message(record), args_copy);

		va_end(args_copy);
	}

	if (body_size <= 0)
	{
		return 0;
	}

	if (buffer_body != NULL)
	{
		char * buffer_end = (((char *)buffer_body) + body_size);

		buffer_end[0] = '\n';
		buffer_end[1] = '\0';
	}

	return (size_t)(header_size + body_size) + 1;
}

static size_t log_policy_format_text_serialize(log_policy policy, const log_record record, void * buffer, const size_t size)
{
	if (log_record_data(record) == NULL)
	{
		return log_policy_format_text_serialize_impl(policy, record, buffer, size);
	}
	else
	{
		return log_policy_format_text_serialize_impl_va(policy, record, buffer, size);
	}
}

static size_t log_policy_format_text_deserialize(log_policy policy, log_record record, const void * buffer, const size_t size)
{
	log_policy_format_text_data text_data = log_policy_instance(policy);

	/* TODO */
	(void)text_data;
	(void)record;
	(void)buffer;
	(void)size;

	return size;
}

static int log_policy_format_text_destroy(log_policy policy)
{
	log_policy_format_text_data text_data = log_policy_instance(policy);

	if (text_data != NULL)
	{
		free(text_data);
	}

	return 0;
}
