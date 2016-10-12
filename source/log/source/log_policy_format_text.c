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

#include <stdio.h>
#include <time.h>

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

LOG_NO_EXPORT static int log_policy_format_text_create(log_policy policy, const log_policy_ctor ctor);

LOG_NO_EXPORT static size_t log_policy_format_text_size(log_policy policy, const log_record record);

LOG_NO_EXPORT static size_t log_policy_format_text_serialize(log_policy policy, const log_record record, void * buffer, const size_t size);

LOG_NO_EXPORT static size_t log_policy_format_text_deserialize(log_policy policy, log_record record, const void * buffer, const size_t size);

LOG_NO_EXPORT static int log_policy_format_text_destroy(log_policy policy);

/* -- Methods -- */

log_policy_interface log_policy_format_text_interface()
{
	static struct log_policy_format_impl_type log_policy_format_text_impl =
	{
		&log_policy_format_text_size,
		&log_policy_format_text_serialize,
		&log_policy_format_text_deserialize
	};

	static struct log_policy_interface_type policy_interface_stream =
	{
		&log_policy_format_text_create,
		&log_policy_format_text_impl,
		&log_policy_format_text_destroy
	};

	return &policy_interface_stream;
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

static size_t log_policy_format_text_serialize(log_policy policy, const log_record record, void * buffer, const size_t size)
{
	static const char format[] = "[%.19s] <%ul> #%ul:%s:%s @%s >> %s\n";

	log_policy_format_text_data text_data = log_policy_instance(policy);

	int result;

	(void)text_data;

	#if defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER < 1900)

		result = _snprintf(buffer, size, format,
			ctime(log_record_time(record)),
			log_record_thread_id(record),
			log_record_line(record),
			log_record_func(record),
			log_record_file(record),
			log_level_name(log_record_level(record)),
			log_record_message(record),
			log_record_data(record));

	#elif defined(_BSD_SOURCE) || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 500) \
		defined(_ISOC99_SOURCE) || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L)

		result = snprintf(buffer, size, format,
			ctime(log_record_time(record)),
			log_record_thread_id(record),
			log_record_line(record),
			log_record_func(record),
			log_record_file(record),
			log_level_name(log_record_level(record)),
			log_record_message(record));

	#else

		/* TODO: find out how to avoid stack smashing */
		(void)size;

		result = sprintf(buffer, format,
			ctime(log_record_time(record)),
			log_record_thread_id(record),
			log_record_line(record),
			log_record_func(record),
			log_record_file(record),
			log_level_name(log_record_level(record)),
			log_record_message(record));

	#endif

	if (result <= 0)
	{
		return 0;
	}

	return (size_t)result;
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
