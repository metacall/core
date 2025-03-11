/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#include <log/log_level.h>
#include <log/log_record.h>

/* -- Member Data -- */

struct log_record_type
{
	time_t time;
	uint64_t id;
	size_t line;
	const char *func;
	const char *file;
	enum log_level_id level;
	const char *message;
	struct log_record_va_list_type *variable_args;
};

/* -- Private Methods -- */

#if defined(LOG_RECORD_FUNCTION_UNKNOWN_IMPL)
const char *__log_record_unknown_function__(void)
{
	static const char __log_record_unknown_function_impl__[] = "unknown_function";

	return __log_record_unknown_function_impl__;
}
#endif

/* -- Protected Methods -- */

size_t log_record_size(void)
{
	return sizeof(struct log_record_type);
}

/* -- Methods -- */

log_record log_record_create(const log_record_ctor record_ctor)
{
	if (record_ctor != NULL)
	{
		log_record record = malloc(sizeof(struct log_record_type));

		if (record != NULL)
		{
			return log_record_initialize(record, record_ctor);
		}
	}

	return NULL;
}

log_record log_record_initialize(log_record record, const log_record_ctor record_ctor)
{
	if (record != NULL && record_ctor != NULL)
	{
		record->time = time(NULL);
		record->id = thread_id_get_current();
		record->line = record_ctor->line;
		record->func = record_ctor->func;
		record->file = record_ctor->file;
		record->level = record_ctor->level;
		record->message = record_ctor->message;
		record->variable_args = record_ctor->variable_args;

		return record;
	}

	return NULL;
}

const time_t *log_record_time(log_record record)
{
	return &record->time;
}

uint64_t log_record_thread_id(log_record record)
{
	return record->id;
}

size_t log_record_line(log_record record)
{
	return record->line;
}

const char *log_record_func(log_record record)
{
	return record->func;
}

const char *log_record_file(log_record record)
{
	return record->file;
}

enum log_level_id log_record_level(log_record record)
{
	return record->level;
}

const char *log_record_message(log_record record)
{
	return record->message;
}

struct log_record_va_list_type *log_record_variable_args(log_record record)
{
	return record->variable_args;
}

int log_record_destroy(log_record record)
{
	if (record != NULL)
	{
		free(record);
	}

	return 0;
}
