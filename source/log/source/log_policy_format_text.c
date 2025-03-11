/*
*	Logger Library by Parra Studios
*	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A generic logger library providing application execution reports.
*
*/

/* -- Headers -- */

#include <log/log_impl.h> /* TODO: Remove by custom log_policy_format_text_data_type instead of impl->level */
#include <log/log_level.h>
#include <log/log_policy_format.h>
#include <log/log_policy_format_text.h>

#include <format/format_print.h>

#include <stdarg.h>
#include <time.h>

/* -- Definitions -- */

#define LOG_POLICY_FORMAT_TEXT_STR_DEBUG   "[%.19s] #%" PRIuS " [ %" PRIuS " | %s | %s ] @%s : "
#define LOG_POLICY_FORMAT_TEXT_STR_RELEASE "[%.19s] #%" PRIuS " @%s : "
#define LOG_POLICY_FORMAT_TEXT_STR_PRETTY  "\x1b[32m%s\x1b[0m: "

/* -- Macros -- */

#ifndef va_copy
	#if defined(__va_copy)
	/* GCC and others define this for older standards compatibility (C89) */
		#define va_copy(dest, src) __va_copy((dest), (src))
	#elif defined(__builtin_va_copy)
		#define va_copy(dest, src) __builtin_va_copy((dest), (src))
	#elif defined(_WIN32) || defined(_WIN64) || \
		defined(_ARCH_PPC) || defined(_POWER) || defined(powerpc) || defined(__powerpc) || \
		defined(__powerpc__) || defined(__PowerPC__) || defined(__POWERPC__) || defined(PPC) || \
		defined(__ppc__) || defined(__PPC) || defined(__PPC__) || \
		defined(_ARCH_PPC64) || defined(__powerpc64__) || defined(__ppc64) || defined(__ppc64__) || defined(__PPC64__)
	/* Works for Microsoft x86, x64 and PowerPC-based platforms */
		#define va_copy(dest, src) ((void)memcpy(&(dest), &(src), sizeof(va_list)))
	#else
		#warning "va_copy may be not supported for this architecture, assuming va_list can be copied as a normal pointer"
		#define va_copy(dest, src) ((dest) = (src))
	#endif
#endif

/* -- Forward Declarations -- */

struct log_policy_format_text_data_type;

/* -- Type Definitions -- */

typedef struct log_policy_format_text_data_type *log_policy_format_text_data;

/* -- Member Data -- */

struct log_policy_format_text_data_type
{
	unsigned int flags;
};

/* -- Private Methods -- */

static int log_policy_format_text_create(log_policy policy, const log_policy_ctor ctor);

static size_t log_policy_format_text_size(log_policy policy, const log_record record);

static size_t log_policy_format_text_serialize(log_policy policy, const log_record record, void *buffer, const size_t size);

static size_t log_policy_format_text_serialize_impl(log_policy policy, const log_record record, void *buffer, const size_t size);

static size_t log_policy_format_text_serialize_impl_va(log_policy policy, const log_record record, void *buffer, const size_t size);

static size_t log_policy_format_text_deserialize(log_policy policy, log_record record, const void *buffer, const size_t size);

static int log_policy_format_text_destroy(log_policy policy);

/* -- Methods -- */

log_policy_interface log_policy_format_text_interface(void)
{
	static struct log_policy_format_impl_type log_policy_format_text_impl_obj = {
		&log_policy_format_text_size,
		&log_policy_format_text_serialize,
		&log_policy_format_text_deserialize
	};

	static struct log_policy_interface_type policy_interface_format = {
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

static const char *log_policy_format_text_serialize_impl_format(enum log_level_id log_level, unsigned int flags)
{
#if (LOG_POLICY_FORMAT_PRETTY == 1)
	{
		(void)log_level;

		if (flags & LOG_POLICY_FORMAT_TEXT_NEWLINE)
		{
			static const char format_debug_newline[] = LOG_POLICY_FORMAT_TEXT_STR_PRETTY "%s\n";

			return format_debug_newline;
		}
		else
		{
			static const char format_debug[] = LOG_POLICY_FORMAT_TEXT_STR_PRETTY "%s";

			return format_debug;
		}
	}
#else
	{
		if (log_level == LOG_LEVEL_DEBUG)
		{
			if (flags & LOG_POLICY_FORMAT_TEXT_NEWLINE)
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
			if (flags & LOG_POLICY_FORMAT_TEXT_NEWLINE)
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
#endif
}

static size_t log_policy_format_text_serialize_impl(log_policy policy, const log_record record, void *buffer, const size_t size)
{
	log_policy_format_text_data text_data = log_policy_instance(policy);

	log_aspect aspect = log_policy_aspect(policy);

	log_impl impl = log_aspect_parent(aspect);

	int length;

	const char *format = log_policy_format_text_serialize_impl_format(log_impl_level(impl), text_data->flags);

#if (LOG_POLICY_FORMAT_PRETTY == 1)
	{
		length = snprintf(buffer, size, format,
			log_level_to_string(log_record_level(record)),
			log_record_message(record));
	}
#else
	{
		if (log_impl_level(impl) == LOG_LEVEL_DEBUG)
		{
			length = snprintf(buffer, size, format,
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
			length = snprintf(buffer, size, format,
				ctime(log_record_time(record)),
				log_record_thread_id(record),
				log_level_to_string(log_record_level(record)),
				log_record_message(record));
		}
	}
#endif

	if (length <= 0)
	{
		return 0;
	}

	return (size_t)length + 1;
}

static size_t log_policy_format_text_serialize_impl_va(log_policy policy, const log_record record, void *buffer, const size_t size)
{
	log_policy_format_text_data text_data = log_policy_instance(policy);

	int header_length = 0, body_length = 0;

	void *buffer_body = NULL;

	struct log_record_va_list_type *variable_args;

#if (LOG_POLICY_FORMAT_PRETTY == 1)
	{
		static const char header_format[] = LOG_POLICY_FORMAT_TEXT_STR_PRETTY;

		header_length = snprintf(buffer, size, header_format,
			log_level_to_string(log_record_level(record)));
	}
#else
	{
		log_aspect aspect = log_policy_aspect(policy);

		log_impl impl = log_aspect_parent(aspect);

		if (log_impl_level(impl) == LOG_LEVEL_DEBUG)
		{
			static const char header_format[] = LOG_POLICY_FORMAT_TEXT_STR_DEBUG;

			header_length = snprintf(buffer, size, header_format,
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

			header_length = snprintf(buffer, size, header_format,
				ctime(log_record_time(record)),
				log_record_thread_id(record),
				log_level_to_string(log_record_level(record)));
		}
	}
#endif

	if (header_length <= 0)
	{
		return 0;
	}

	if (buffer != NULL)
	{
		buffer_body = (void *)(((char *)buffer) + header_length);
	}

	variable_args = log_record_variable_args(record);

	if (variable_args != NULL)
	{
		va_list args_copy;

		va_copy(args_copy, variable_args->data);

		body_length = vsnprintf(buffer_body, size, log_record_message(record), args_copy);

		va_end(args_copy);
	}

	if (body_length <= 0)
	{
		return 0;
	}

	if (text_data->flags & LOG_POLICY_FORMAT_TEXT_NEWLINE)
	{
		if (buffer_body != NULL)
		{
			char *buffer_body_str = (char *)buffer_body;

			buffer_body_str[body_length] = '\n';
			buffer_body_str[body_length + 1] = '\0';
		}

		++body_length;
	}

	return (size_t)(header_length + body_length + 1);
}

static size_t log_policy_format_text_serialize(log_policy policy, const log_record record, void *buffer, const size_t size)
{
	if (log_record_variable_args(record) == NULL)
	{
		return log_policy_format_text_serialize_impl(policy, record, buffer, size);
	}
	else
	{
		return log_policy_format_text_serialize_impl_va(policy, record, buffer, size);
	}
}

static size_t log_policy_format_text_deserialize(log_policy policy, log_record record, const void *buffer, const size_t size)
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
