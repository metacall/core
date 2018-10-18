/*
*	Logger Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A generic logger library providing application execution reports.
*
*/

/* -- Headers -- */

#include <log/log_policy_format_text.h>
#include <log/log_policy_format.h>
#include <log/log_level.h>
#include <log/log_impl.h> /* TODO: Remove by custom log_policy_format_text_data_type instead of impl->level */

#include <format/format_print.h>

#include <time.h>

/* -- Definitions -- */

/* TODO: Check why this +1 is happening on windows, solve the problem in the behavior and delete this macros */

#if defined(_WIN32)
#	if defined(_MSC_VER) && (_MSC_VER < 1900)
#		define format_snprintf(buffer, count, format, ...) (snprintf(buffer, count, format, __VA_ARGS__) + 1)
#		define format_vsnprintf(buffer, count, format, arg_list) (vsnprintf(buffer, count, format, arg_list) + 1)
#	else
#		define format_snprintf(buffer, count, format, ...) (snprintf(buffer, count, format, __VA_ARGS__) + 1)
#		define format_vsnprintf(buffer, count, format, arg_list) (vsnprintf(buffer, count, format, arg_list) + 1)
#	endif
#elif defined(_BSD_SOURCE) || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 500) || \
	defined(_ISOC99_SOURCE) || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L)
#	define format_snprintf(buffer, count, format, ...) snprintf(buffer, count, format, __VA_ARGS__)
#	define format_vsnprintf(buffer, count, format, arg_list) vsnprintf(buffer, count, format, arg_list)
#endif

#define LOG_POLICY_FORMAT_TEXT_STR_DEBUG "[%.19s] #%" PRIuS " [ %" PRIuS " | %s | %s ] @%s : "
#define LOG_POLICY_FORMAT_TEXT_STR_RELEASE "[%.19s] #%" PRIuS " @%s : "

/* -- Forward Declarations -- */

struct log_policy_format_text_data_type;

/* -- Type Definitions -- */

typedef struct log_policy_format_text_data_type * log_policy_format_text_data;

/* -- Member Data -- */

struct log_policy_format_text_data_type
{
	unsigned int flags;
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

	const log_policy_format_text_ctor text_ctor = ctor;

	if (text_data == NULL)
	{
		return 1;
	}

	if (text_ctor != NULL)
	{
		text_data->flags = text_ctor->flags;
	}
	else
	{
		text_data->flags = LOG_POLICY_FORMAT_TEXT_EMPTY;
	}

	text_data->flags = text_ctor->flags;

	log_policy_instantiate(policy, text_data, LOG_POLICY_FORMAT_TEXT);

	return 0;
}

static size_t log_policy_format_text_size(log_policy policy, const log_record record)
{
	return log_policy_format_text_serialize(policy, record, NULL, 0);
}

static const char * log_policy_format_text_serialize_impl_format(enum log_level_id log_level, unsigned int flags)
{
	if (log_level == LOG_LEVEL_DEBUG)
	{
		if (flags | LOG_POLICY_FORMAT_TEXT_NEWLINE)
		{
			static const char format_debug_newline[] = LOG_POLICY_FORMAT_TEXT_STR_DEBUG "%s\n";

			return format_debug_newline;
		}
		else
		{
			static const char format_debug[] = LOG_POLICY_FORMAT_TEXT_STR_DEBUG "%s";

			return format_debug;
		}
	}
	else
	{
		if (flags | LOG_POLICY_FORMAT_TEXT_NEWLINE)
		{
			static const char format_release_newline[] = LOG_POLICY_FORMAT_TEXT_STR_RELEASE "%s\n";

			return format_release_newline;
		}
		else
		{
			static const char format_release[] = LOG_POLICY_FORMAT_TEXT_STR_RELEASE "%s";

			return format_release;
		}
	}
}

static size_t log_policy_format_text_serialize_impl(log_policy policy, const log_record record, void * buffer, const size_t size)
{
	log_policy_format_text_data text_data = log_policy_instance(policy);

	log_aspect aspect = log_policy_aspect(policy);

	log_impl impl = log_aspect_parent(aspect);

	int result;

	const char * format = log_policy_format_text_serialize_impl_format(log_impl_level(impl), text_data->flags);

	if (log_impl_level(impl) == LOG_LEVEL_DEBUG)
	{
		result = format_snprintf(buffer, size, format,
			ctime(log_record_time(record)),
			log_record_thread_id(record),
			log_record_line(record),
			log_record_func(record),
			log_record_file(record),
			log_level_to_string(log_record_level(record)),
			log_record_message(record));
	}
	else
	{
		result = format_snprintf(buffer, size, format,
			ctime(log_record_time(record)),
			log_record_thread_id(record),
			log_level_to_string(log_record_level(record)),
			log_record_message(record));
	}

	if (result <= 0)
	{
		return 0;
	}

	return (size_t)result;
}

static size_t log_policy_format_text_serialize_impl_va(log_policy policy, const log_record record, void * buffer, const size_t size)
{
	log_policy_format_text_data text_data = log_policy_instance(policy);

	log_aspect aspect = log_policy_aspect(policy);

	log_impl impl = log_aspect_parent(aspect);

	int header_size = 0, body_size = 0;

	void * buffer_body = NULL;

	if (log_impl_level(impl) == LOG_LEVEL_DEBUG)
	{
		static const char header_format[] = LOG_POLICY_FORMAT_TEXT_STR_DEBUG;

		header_size = format_snprintf(buffer, size, header_format,
			ctime(log_record_time(record)),
			log_record_thread_id(record),
			log_record_line(record),
			log_record_func(record),
			log_record_file(record),
			log_level_to_string(log_record_level(record)));
	}
	else
	{
		static const char header_format[] = LOG_POLICY_FORMAT_TEXT_STR_RELEASE;

		header_size = format_snprintf(buffer, size, header_format,
			ctime(log_record_time(record)),
			log_record_thread_id(record),
			log_level_to_string(log_record_level(record)));
	}

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

		body_size = format_vsnprintf(buffer_body, size, log_record_message(record), args_copy);

		va_end(args_copy);
	}

	if (body_size <= 0)
	{
		return 0;
	}

	if (text_data->flags | LOG_POLICY_FORMAT_TEXT_NEWLINE)
	{
		if (buffer_body != NULL)
		{
			char * buffer_body_str = (char *)buffer_body;

			buffer_body_str[body_size] = '\n';
			buffer_body_str[body_size + 1] = '\0';
		}

		++body_size;
	}

	return (size_t)(header_size + body_size);
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
