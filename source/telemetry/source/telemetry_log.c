/*
 *	Telemetry Library by MetaCall Inc.
 *	A library for logging and telemetry collection in MetaCall runtime.
 *
 *	Copyright (C) 2025 MetaCall Inc., Dhiren Mhatre
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* -- Headers -- */

#include <telemetry/telemetry_log.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
	#include <process.h>
	#include <windows.h>
	#define getpid _getpid
	#define snprintf _snprintf
#else
	#include <pthread.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <unistd.h>
#endif

/* -- Private Data -- */

static struct telemetry_log_context_type global_context = {
	.handler_count = 0,
	.filter_count = 0,
	.global_level = TELEMETRY_LOG_LEVEL_INFO,
	.async_logging = 0,
	.async_queue = NULL,
	.mutex = NULL
};

static int telemetry_log_initialized = 0;

/* -- Private Forward Declarations -- */

static int telemetry_log_entry_format_text(telemetry_log_entry entry, char *buffer, size_t size, telemetry_log_formatter formatter);
static int telemetry_log_entry_format_json(telemetry_log_entry entry, char *buffer, size_t size, telemetry_log_formatter formatter);
static int telemetry_log_entry_format_xml(telemetry_log_entry entry, char *buffer, size_t size, telemetry_log_formatter formatter);
static int telemetry_log_entry_format_colored(telemetry_log_entry entry, char *buffer, size_t size, telemetry_log_formatter formatter);
static int telemetry_log_handler_process(telemetry_log_handler handler, telemetry_log_entry entry);
static int telemetry_log_filter_match(telemetry_log_filter filter, telemetry_log_entry entry);
static void telemetry_log_get_timestamp(char *buffer, size_t size);
static uint64_t telemetry_log_get_thread_id(void);
static const char *telemetry_log_get_color_code(telemetry_log_level_id level);
static const char *telemetry_log_get_reset_code(void);
static int telemetry_log_rotate_file(telemetry_log_handler handler);
static int telemetry_log_should_rotate(telemetry_log_handler handler);

/* -- Methods -- */

int telemetry_log_initialize(void)
{
	if (telemetry_log_initialized)
	{
		return 0;
	}

	/* Initialize the global context */
	memset(&global_context, 0, sizeof(struct telemetry_log_context_type));
	global_context.global_level = TELEMETRY_LOG_LEVEL_INFO;
	global_context.async_logging = 0;

	/* TODO: Initialize mutex for thread safety */
	/* TODO: Initialize async queue if needed */

	telemetry_log_initialized = 1;

	return 0;
}

int telemetry_log_shutdown(void)
{
	size_t i;

	if (!telemetry_log_initialized)
	{
		return 0;
	}

	/* Flush all pending logs */
	telemetry_log_flush();

	/* Destroy all handlers */
	for (i = 0; i < global_context.handler_count; ++i)
	{
		if (global_context.handlers[i] != NULL)
		{
			telemetry_log_handler_destroy(global_context.handlers[i]);
			global_context.handlers[i] = NULL;
		}
	}

	/* Destroy all filters */
	for (i = 0; i < global_context.filter_count; ++i)
	{
		if (global_context.filters[i] != NULL)
		{
			telemetry_log_filter_destroy(global_context.filters[i]);
			global_context.filters[i] = NULL;
		}
	}

	/* TODO: Destroy mutex */
	/* TODO: Destroy async queue */

	telemetry_log_initialized = 0;

	return 0;
}

telemetry_log_context telemetry_log_get_context(void)
{
	return &global_context;
}

int telemetry_log_set_level(telemetry_log_level_id level)
{
	if (level < TELEMETRY_LOG_LEVEL_TRACE || level > TELEMETRY_LOG_LEVEL_CRITICAL)
	{
		return 1;
	}

	global_context.global_level = level;
	return 0;
}

telemetry_log_level_id telemetry_log_get_level(void)
{
	return global_context.global_level;
}

telemetry_log_handler telemetry_log_handler_create(telemetry_log_handler_type_id type, telemetry_log_level_id min_level)
{
	telemetry_log_handler handler = (telemetry_log_handler)malloc(sizeof(struct telemetry_log_handler_type));

	if (handler == NULL)
	{
		return NULL;
	}

	memset(handler, 0, sizeof(struct telemetry_log_handler_type));

	handler->type = type;
	handler->min_level = min_level;
	handler->callback = NULL;
	handler->formatter = NULL;
	handler->user_data = NULL;
	handler->file_handle = NULL;
	handler->file_path = NULL;
	handler->rotation = TELEMETRY_LOG_ROTATION_NONE;
	handler->max_file_size = 0;
	handler->enabled = 1;

	return handler;
}

void telemetry_log_handler_destroy(telemetry_log_handler handler)
{
	if (handler == NULL)
	{
		return;
	}

	/* Close file handle if open */
	if (handler->file_handle != NULL && handler->file_handle != stdout && handler->file_handle != stderr)
	{
		fclose(handler->file_handle);
	}

	/* Free file path */
	if (handler->file_path != NULL)
	{
		free(handler->file_path);
	}

	/* Destroy formatter if present */
	if (handler->formatter != NULL)
	{
		telemetry_log_formatter_destroy(handler->formatter);
	}

	free(handler);
}

int telemetry_log_handler_register(telemetry_log_handler handler)
{
	if (handler == NULL)
	{
		return 1;
	}

	if (global_context.handler_count >= TELEMETRY_LOG_MAX_HANDLERS)
	{
		return 1;
	}

	global_context.handlers[global_context.handler_count++] = handler;

	return 0;
}

int telemetry_log_handler_unregister(telemetry_log_handler handler)
{
	size_t i, j;

	if (handler == NULL)
	{
		return 1;
	}

	for (i = 0; i < global_context.handler_count; ++i)
	{
		if (global_context.handlers[i] == handler)
		{
			/* Shift remaining handlers */
			for (j = i; j < global_context.handler_count - 1; ++j)
			{
				global_context.handlers[j] = global_context.handlers[j + 1];
			}

			global_context.handler_count--;
			return 0;
		}
	}

	return 1;
}

int telemetry_log_handler_set_callback(telemetry_log_handler handler, telemetry_log_handler_callback callback, void *user_data)
{
	if (handler == NULL || callback == NULL)
	{
		return 1;
	}

	handler->callback = callback;
	handler->user_data = user_data;

	return 0;
}

int telemetry_log_handler_configure_file(telemetry_log_handler handler, const char *file_path, telemetry_log_rotation_policy_id rotation, size_t max_size)
{
	if (handler == NULL || file_path == NULL)
	{
		return 1;
	}

	if (handler->type != TELEMETRY_LOG_HANDLER_FILE)
	{
		return 1;
	}

	/* Close existing file if open */
	if (handler->file_handle != NULL)
	{
		fclose(handler->file_handle);
		handler->file_handle = NULL;
	}

	/* Free existing path */
	if (handler->file_path != NULL)
	{
		free(handler->file_path);
	}

	/* Allocate and copy new path */
	handler->file_path = (char *)malloc(strlen(file_path) + 1);
	if (handler->file_path == NULL)
	{
		return 1;
	}
	strcpy(handler->file_path, file_path);

	/* Open the file */
	handler->file_handle = fopen(file_path, "a");
	if (handler->file_handle == NULL)
	{
		free(handler->file_path);
		handler->file_path = NULL;
		return 1;
	}

	handler->rotation = rotation;
	handler->max_file_size = max_size;

	return 0;
}

telemetry_log_formatter telemetry_log_formatter_create(telemetry_log_format_id format)
{
	telemetry_log_formatter formatter = (telemetry_log_formatter)malloc(sizeof(struct telemetry_log_formatter_type));

	if (formatter == NULL)
	{
		return NULL;
	}

	memset(formatter, 0, sizeof(struct telemetry_log_formatter_type));

	formatter->format = format;
	formatter->callback = NULL;
	formatter->user_data = NULL;
	formatter->include_timestamp = 1;
	formatter->include_level = 1;
	formatter->include_category = 1;
	formatter->include_location = 1;
	formatter->include_thread_info = 0;

	return formatter;
}

void telemetry_log_formatter_destroy(telemetry_log_formatter formatter)
{
	if (formatter == NULL)
	{
		return;
	}

	free(formatter);
}

int telemetry_log_formatter_configure(telemetry_log_formatter formatter, int include_timestamp, int include_level, int include_category, int include_location, int include_thread_info)
{
	if (formatter == NULL)
	{
		return 1;
	}

	formatter->include_timestamp = include_timestamp;
	formatter->include_level = include_level;
	formatter->include_category = include_category;
	formatter->include_location = include_location;
	formatter->include_thread_info = include_thread_info;

	return 0;
}

int telemetry_log_formatter_set_callback(telemetry_log_formatter formatter, telemetry_log_formatter_callback callback, void *user_data)
{
	if (formatter == NULL || callback == NULL)
	{
		return 1;
	}

	formatter->callback = callback;
	formatter->user_data = user_data;

	return 0;
}

int telemetry_log_handler_set_formatter(telemetry_log_handler handler, telemetry_log_formatter formatter)
{
	if (handler == NULL || formatter == NULL)
	{
		return 1;
	}

	handler->formatter = formatter;

	return 0;
}

telemetry_log_filter telemetry_log_filter_create(void)
{
	telemetry_log_filter filter = (telemetry_log_filter)malloc(sizeof(struct telemetry_log_filter_type));

	if (filter == NULL)
	{
		return NULL;
	}

	memset(filter, 0, sizeof(struct telemetry_log_filter_type));

	filter->callback = NULL;
	filter->user_data = NULL;
	filter->category_pattern[0] = '\0';
	filter->min_level = TELEMETRY_LOG_LEVEL_TRACE;
	filter->enabled = 1;

	return filter;
}

void telemetry_log_filter_destroy(telemetry_log_filter filter)
{
	if (filter == NULL)
	{
		return;
	}

	free(filter);
}

int telemetry_log_filter_register(telemetry_log_filter filter)
{
	if (filter == NULL)
	{
		return 1;
	}

	if (global_context.filter_count >= TELEMETRY_LOG_MAX_FILTERS)
	{
		return 1;
	}

	global_context.filters[global_context.filter_count++] = filter;

	return 0;
}

int telemetry_log_filter_unregister(telemetry_log_filter filter)
{
	size_t i, j;

	if (filter == NULL)
	{
		return 1;
	}

	for (i = 0; i < global_context.filter_count; ++i)
	{
		if (global_context.filters[i] == filter)
		{
			/* Shift remaining filters */
			for (j = i; j < global_context.filter_count - 1; ++j)
			{
				global_context.filters[j] = global_context.filters[j + 1];
			}

			global_context.filter_count--;
			return 0;
		}
	}

	return 1;
}

int telemetry_log_filter_set_callback(telemetry_log_filter filter, telemetry_log_filter_callback callback, void *user_data)
{
	if (filter == NULL || callback == NULL)
	{
		return 1;
	}

	filter->callback = callback;
	filter->user_data = user_data;

	return 0;
}

int telemetry_log_filter_set_category(telemetry_log_filter filter, const char *pattern)
{
	if (filter == NULL || pattern == NULL)
	{
		return 1;
	}

	strncpy(filter->category_pattern, pattern, TELEMETRY_LOG_CATEGORY_SIZE - 1);
	filter->category_pattern[TELEMETRY_LOG_CATEGORY_SIZE - 1] = '\0';

	return 0;
}

int telemetry_log_filter_set_level(telemetry_log_filter filter, telemetry_log_level_id level)
{
	if (filter == NULL)
	{
		return 1;
	}

	if (level < TELEMETRY_LOG_LEVEL_TRACE || level > TELEMETRY_LOG_LEVEL_CRITICAL)
	{
		return 1;
	}

	filter->min_level = level;

	return 0;
}

int telemetry_log_write(telemetry_log_level_id level, const char *category, const char *file, const char *function, int line, const char *format, ...)
{
	struct telemetry_log_entry_type entry;
	va_list args;
	size_t i;
	int should_log = 0;

	/* Check if logging is initialized */
	if (!telemetry_log_initialized)
	{
		telemetry_log_initialize();
	}

	/* Check global log level */
	if (level < global_context.global_level)
	{
		return 0;
	}

	/* Initialize entry */
	memset(&entry, 0, sizeof(struct telemetry_log_entry_type));
	entry.level = level;
	entry.file = file;
	entry.function = function;
	entry.line = line;
	entry.time = time(NULL);
	entry.thread_id = telemetry_log_get_thread_id();
	entry.process_id = (uint32_t)getpid();

	/* Copy category */
	if (category != NULL)
	{
		strncpy(entry.category, category, TELEMETRY_LOG_CATEGORY_SIZE - 1);
		entry.category[TELEMETRY_LOG_CATEGORY_SIZE - 1] = '\0';
	}
	else
	{
		strcpy(entry.category, "general");
	}

	/* Format message */
	va_start(args, format);
	vsnprintf(entry.message, TELEMETRY_LOG_MAX_MESSAGE_SIZE, format, args);
	va_end(args);

	/* Generate timestamp */
	telemetry_log_get_timestamp(entry.timestamp, TELEMETRY_LOG_TIMESTAMP_SIZE);

	/* Apply filters */
	should_log = 1;
	for (i = 0; i < global_context.filter_count; ++i)
	{
		if (global_context.filters[i] != NULL && global_context.filters[i]->enabled)
		{
			if (!telemetry_log_filter_match(global_context.filters[i], &entry))
			{
				should_log = 0;
				break;
			}
		}
	}

	if (!should_log)
	{
		return 0;
	}

	/* Process with all handlers */
	return telemetry_log_write_entry(&entry);
}

int telemetry_log_write_entry(telemetry_log_entry entry)
{
	size_t i;
	int result = 0;

	if (entry == NULL)
	{
		return 1;
	}

	/* TODO: If async logging is enabled, queue the entry */
	if (global_context.async_logging)
	{
		/* Queue entry for async processing */
		/* For now, fall through to synchronous processing */
	}

	/* Process with all registered handlers */
	for (i = 0; i < global_context.handler_count; ++i)
	{
		if (global_context.handlers[i] != NULL && global_context.handlers[i]->enabled)
		{
			if (entry->level >= global_context.handlers[i]->min_level)
			{
				int handler_result = telemetry_log_handler_process(global_context.handlers[i], entry);
				if (handler_result != 0)
				{
					result = handler_result;
				}
			}
		}
	}

	return result;
}

int telemetry_log_set_async(int async)
{
	global_context.async_logging = async;

	/* TODO: Initialize or destroy async queue based on flag */

	return 0;
}

int telemetry_log_flush(void)
{
	size_t i;

	/* TODO: Flush async queue if present */

	/* Flush all file handlers */
	for (i = 0; i < global_context.handler_count; ++i)
	{
		if (global_context.handlers[i] != NULL)
		{
			telemetry_log_handler handler = global_context.handlers[i];
			if (handler->file_handle != NULL)
			{
				fflush(handler->file_handle);
			}
		}
	}

	return 0;
}

const char *telemetry_log_level_to_string(telemetry_log_level_id level)
{
	switch (level)
	{
		case TELEMETRY_LOG_LEVEL_TRACE:
			return "TRACE";
		case TELEMETRY_LOG_LEVEL_DEBUG:
			return "DEBUG";
		case TELEMETRY_LOG_LEVEL_INFO:
			return "INFO";
		case TELEMETRY_LOG_LEVEL_WARNING:
			return "WARNING";
		case TELEMETRY_LOG_LEVEL_ERROR:
			return "ERROR";
		case TELEMETRY_LOG_LEVEL_CRITICAL:
			return "CRITICAL";
		default:
			return "UNKNOWN";
	}
}

telemetry_log_level_id telemetry_log_level_from_string(const char *str)
{
	if (str == NULL)
	{
		return TELEMETRY_LOG_LEVEL_INFO;
	}

	if (strcmp(str, "TRACE") == 0 || strcmp(str, "trace") == 0)
	{
		return TELEMETRY_LOG_LEVEL_TRACE;
	}
	else if (strcmp(str, "DEBUG") == 0 || strcmp(str, "debug") == 0)
	{
		return TELEMETRY_LOG_LEVEL_DEBUG;
	}
	else if (strcmp(str, "INFO") == 0 || strcmp(str, "info") == 0)
	{
		return TELEMETRY_LOG_LEVEL_INFO;
	}
	else if (strcmp(str, "WARNING") == 0 || strcmp(str, "warning") == 0 || strcmp(str, "WARN") == 0 || strcmp(str, "warn") == 0)
	{
		return TELEMETRY_LOG_LEVEL_WARNING;
	}
	else if (strcmp(str, "ERROR") == 0 || strcmp(str, "error") == 0)
	{
		return TELEMETRY_LOG_LEVEL_ERROR;
	}
	else if (strcmp(str, "CRITICAL") == 0 || strcmp(str, "critical") == 0 || strcmp(str, "CRIT") == 0 || strcmp(str, "crit") == 0)
	{
		return TELEMETRY_LOG_LEVEL_CRITICAL;
	}

	return TELEMETRY_LOG_LEVEL_INFO;
}

/* -- Private Methods -- */

static int telemetry_log_entry_format_text(telemetry_log_entry entry, char *buffer, size_t size, telemetry_log_formatter formatter)
{
	int written = 0;
	int n;

	if (entry == NULL || buffer == NULL || size == 0)
	{
		return -1;
	}

	buffer[0] = '\0';

	/* Timestamp */
	if (formatter->include_timestamp)
	{
		n = snprintf(buffer + written, size - written, "[%s] ", entry->timestamp);
		if (n > 0)
			written += n;
	}

	/* Log level */
	if (formatter->include_level)
	{
		n = snprintf(buffer + written, size - written, "[%s] ", telemetry_log_level_to_string(entry->level));
		if (n > 0)
			written += n;
	}

	/* Category */
	if (formatter->include_category && entry->category[0] != '\0')
	{
		n = snprintf(buffer + written, size - written, "[%s] ", entry->category);
		if (n > 0)
			written += n;
	}

	/* Thread info */
	if (formatter->include_thread_info)
	{
		n = snprintf(buffer + written, size - written, "[pid:%u tid:%llu] ", entry->process_id, (unsigned long long)entry->thread_id);
		if (n > 0)
			written += n;
	}

	/* Message */
	n = snprintf(buffer + written, size - written, "%s", entry->message);
	if (n > 0)
		written += n;

	/* Location */
	if (formatter->include_location && entry->file != NULL)
	{
		n = snprintf(buffer + written, size - written, " (%s:%d in %s)", entry->file, entry->line, entry->function ? entry->function : "?");
		if (n > 0)
			written += n;
	}

	return written;
}

static int telemetry_log_entry_format_json(telemetry_log_entry entry, char *buffer, size_t size, telemetry_log_formatter formatter)
{
	int written = 0;
	int n;

	if (entry == NULL || buffer == NULL || size == 0)
	{
		return -1;
	}

	n = snprintf(buffer + written, size - written, "{");
	if (n > 0)
		written += n;

	if (formatter->include_timestamp)
	{
		n = snprintf(buffer + written, size - written, "\"timestamp\":\"%s\",", entry->timestamp);
		if (n > 0)
			written += n;
	}

	if (formatter->include_level)
	{
		n = snprintf(buffer + written, size - written, "\"level\":\"%s\",", telemetry_log_level_to_string(entry->level));
		if (n > 0)
			written += n;
	}

	if (formatter->include_category)
	{
		n = snprintf(buffer + written, size - written, "\"category\":\"%s\",", entry->category);
		if (n > 0)
			written += n;
	}

	if (formatter->include_thread_info)
	{
		n = snprintf(buffer + written, size - written, "\"process_id\":%u,\"thread_id\":%llu,", entry->process_id, (unsigned long long)entry->thread_id);
		if (n > 0)
			written += n;
	}

	n = snprintf(buffer + written, size - written, "\"message\":\"%s\"", entry->message);
	if (n > 0)
		written += n;

	if (formatter->include_location && entry->file != NULL)
	{
		n = snprintf(buffer + written, size - written, ",\"file\":\"%s\",\"line\":%d,\"function\":\"%s\"", entry->file, entry->line, entry->function ? entry->function : "?");
		if (n > 0)
			written += n;
	}

	n = snprintf(buffer + written, size - written, "}");
	if (n > 0)
		written += n;

	return written;
}

static int telemetry_log_entry_format_xml(telemetry_log_entry entry, char *buffer, size_t size, telemetry_log_formatter formatter)
{
	int written = 0;
	int n;

	if (entry == NULL || buffer == NULL || size == 0)
	{
		return -1;
	}

	n = snprintf(buffer + written, size - written, "<log>");
	if (n > 0)
		written += n;

	if (formatter->include_timestamp)
	{
		n = snprintf(buffer + written, size - written, "<timestamp>%s</timestamp>", entry->timestamp);
		if (n > 0)
			written += n;
	}

	if (formatter->include_level)
	{
		n = snprintf(buffer + written, size - written, "<level>%s</level>", telemetry_log_level_to_string(entry->level));
		if (n > 0)
			written += n;
	}

	if (formatter->include_category)
	{
		n = snprintf(buffer + written, size - written, "<category>%s</category>", entry->category);
		if (n > 0)
			written += n;
	}

	if (formatter->include_thread_info)
	{
		n = snprintf(buffer + written, size - written, "<process_id>%u</process_id><thread_id>%llu</thread_id>", entry->process_id, (unsigned long long)entry->thread_id);
		if (n > 0)
			written += n;
	}

	n = snprintf(buffer + written, size - written, "<message>%s</message>", entry->message);
	if (n > 0)
		written += n;

	if (formatter->include_location && entry->file != NULL)
	{
		n = snprintf(buffer + written, size - written, "<location><file>%s</file><line>%d</line><function>%s</function></location>", entry->file, entry->line, entry->function ? entry->function : "?");
		if (n > 0)
			written += n;
	}

	n = snprintf(buffer + written, size - written, "</log>");
	if (n > 0)
		written += n;

	return written;
}

static int telemetry_log_entry_format_colored(telemetry_log_entry entry, char *buffer, size_t size, telemetry_log_formatter formatter)
{
	int written = 0;
	int n;
	const char *color_code = telemetry_log_get_color_code(entry->level);
	const char *reset_code = telemetry_log_get_reset_code();

	if (entry == NULL || buffer == NULL || size == 0)
	{
		return -1;
	}

	buffer[0] = '\0';

	/* Start with color */
	n = snprintf(buffer + written, size - written, "%s", color_code);
	if (n > 0)
		written += n;

	/* Timestamp */
	if (formatter->include_timestamp)
	{
		n = snprintf(buffer + written, size - written, "[%s] ", entry->timestamp);
		if (n > 0)
			written += n;
	}

	/* Log level */
	if (formatter->include_level)
	{
		n = snprintf(buffer + written, size - written, "[%s] ", telemetry_log_level_to_string(entry->level));
		if (n > 0)
			written += n;
	}

	/* Reset color after level */
	n = snprintf(buffer + written, size - written, "%s", reset_code);
	if (n > 0)
		written += n;

	/* Category */
	if (formatter->include_category && entry->category[0] != '\0')
	{
		n = snprintf(buffer + written, size - written, "[%s] ", entry->category);
		if (n > 0)
			written += n;
	}

	/* Thread info */
	if (formatter->include_thread_info)
	{
		n = snprintf(buffer + written, size - written, "[pid:%u tid:%llu] ", entry->process_id, (unsigned long long)entry->thread_id);
		if (n > 0)
			written += n;
	}

	/* Message */
	n = snprintf(buffer + written, size - written, "%s", entry->message);
	if (n > 0)
		written += n;

	/* Location */
	if (formatter->include_location && entry->file != NULL)
	{
		n = snprintf(buffer + written, size - written, " (%s:%d in %s)", entry->file, entry->line, entry->function ? entry->function : "?");
		if (n > 0)
			written += n;
	}

	return written;
}

static int telemetry_log_handler_process(telemetry_log_handler handler, telemetry_log_entry entry)
{
	char formatted_buffer[TELEMETRY_LOG_MAX_MESSAGE_SIZE * 2];
	int formatted_length;

	if (handler == NULL || entry == NULL)
	{
		return 1;
	}

	/* Check file rotation if needed */
	if (handler->type == TELEMETRY_LOG_HANDLER_FILE && handler->rotation != TELEMETRY_LOG_ROTATION_NONE)
	{
		if (telemetry_log_should_rotate(handler))
		{
			telemetry_log_rotate_file(handler);
		}
	}

	/* Use custom callback if provided */
	if (handler->callback != NULL)
	{
		return handler->callback(entry, handler->user_data);
	}

	/* Format the log entry */
	if (handler->formatter != NULL)
	{
		if (handler->formatter->callback != NULL)
		{
			formatted_length = handler->formatter->callback(entry, formatted_buffer, sizeof(formatted_buffer), handler->formatter->user_data);
		}
		else
		{
			switch (handler->formatter->format)
			{
				case TELEMETRY_LOG_FORMAT_JSON:
					formatted_length = telemetry_log_entry_format_json(entry, formatted_buffer, sizeof(formatted_buffer), handler->formatter);
					break;
				case TELEMETRY_LOG_FORMAT_XML:
					formatted_length = telemetry_log_entry_format_xml(entry, formatted_buffer, sizeof(formatted_buffer), handler->formatter);
					break;
				case TELEMETRY_LOG_FORMAT_COLORED:
					formatted_length = telemetry_log_entry_format_colored(entry, formatted_buffer, sizeof(formatted_buffer), handler->formatter);
					break;
				case TELEMETRY_LOG_FORMAT_TEXT:
				default:
					formatted_length = telemetry_log_entry_format_text(entry, formatted_buffer, sizeof(formatted_buffer), handler->formatter);
					break;
			}
		}
	}
	else
	{
		/* Default text format */
		telemetry_log_formatter default_formatter_struct;
		default_formatter_struct.format = TELEMETRY_LOG_FORMAT_TEXT;
		default_formatter_struct.include_timestamp = 1;
		default_formatter_struct.include_level = 1;
		default_formatter_struct.include_category = 1;
		default_formatter_struct.include_location = 1;
		default_formatter_struct.include_thread_info = 0;

		formatted_length = telemetry_log_entry_format_text(entry, formatted_buffer, sizeof(formatted_buffer), &default_formatter_struct);
	}

	if (formatted_length < 0)
	{
		return 1;
	}

	/* Output to the appropriate destination */
	switch (handler->type)
	{
		case TELEMETRY_LOG_HANDLER_CONSOLE:
			if (entry->level >= TELEMETRY_LOG_LEVEL_ERROR)
			{
				fprintf(stderr, "%s\n", formatted_buffer);
			}
			else
			{
				fprintf(stdout, "%s\n", formatted_buffer);
			}
			break;

		case TELEMETRY_LOG_HANDLER_FILE:
			if (handler->file_handle != NULL)
			{
				fprintf(handler->file_handle, "%s\n", formatted_buffer);
				fflush(handler->file_handle);
			}
			break;

		case TELEMETRY_LOG_HANDLER_SYSLOG:
			/* TODO: Implement syslog handler */
			break;

		case TELEMETRY_LOG_HANDLER_NETWORK:
			/* TODO: Implement network handler */
			break;

		case TELEMETRY_LOG_HANDLER_CUSTOM:
			/* Custom handlers must provide a callback */
			break;

		default:
			return 1;
	}

	return 0;
}

static int telemetry_log_filter_match(telemetry_log_filter filter, telemetry_log_entry entry)
{
	if (filter == NULL || entry == NULL)
	{
		return 0;
	}

	/* Check level */
	if (entry->level < filter->min_level)
	{
		return 0;
	}

	/* Check category pattern if set */
	if (filter->category_pattern[0] != '\0')
	{
		/* Simple wildcard matching (simplified) */
		if (strcmp(filter->category_pattern, "*") != 0)
		{
			if (strstr(entry->category, filter->category_pattern) == NULL)
			{
				return 0;
			}
		}
	}

	/* Use custom callback if provided */
	if (filter->callback != NULL)
	{
		return filter->callback(entry, filter->user_data);
	}

	return 1;
}

static void telemetry_log_get_timestamp(char *buffer, size_t size)
{
	time_t now;
	struct tm *tm_info;

	if (buffer == NULL || size == 0)
	{
		return;
	}

	time(&now);
	tm_info = localtime(&now);

	strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

static uint64_t telemetry_log_get_thread_id(void)
{
#if defined(_WIN32) || defined(_WIN64)
	return (uint64_t)GetCurrentThreadId();
#else
	return (uint64_t)pthread_self();
#endif
}

static const char *telemetry_log_get_color_code(telemetry_log_level_id level)
{
	switch (level)
	{
		case TELEMETRY_LOG_LEVEL_TRACE:
			return "\033[0;37m"; /* White */
		case TELEMETRY_LOG_LEVEL_DEBUG:
			return "\033[0;36m"; /* Cyan */
		case TELEMETRY_LOG_LEVEL_INFO:
			return "\033[0;32m"; /* Green */
		case TELEMETRY_LOG_LEVEL_WARNING:
			return "\033[0;33m"; /* Yellow */
		case TELEMETRY_LOG_LEVEL_ERROR:
			return "\033[0;31m"; /* Red */
		case TELEMETRY_LOG_LEVEL_CRITICAL:
			return "\033[1;31m"; /* Bold Red */
		default:
			return "\033[0m"; /* Reset */
	}
}

static const char *telemetry_log_get_reset_code(void)
{
	return "\033[0m";
}

static int telemetry_log_rotate_file(telemetry_log_handler handler)
{
	char rotated_path[512];
	time_t now;
	struct tm *tm_info;

	if (handler == NULL || handler->file_path == NULL)
	{
		return 1;
	}

	/* Close current file */
	if (handler->file_handle != NULL)
	{
		fclose(handler->file_handle);
		handler->file_handle = NULL;
	}

	/* Generate rotated filename with timestamp */
	time(&now);
	tm_info = localtime(&now);
	snprintf(rotated_path, sizeof(rotated_path), "%s.%04d%02d%02d_%02d%02d%02d",
		handler->file_path,
		tm_info->tm_year + 1900,
		tm_info->tm_mon + 1,
		tm_info->tm_mday,
		tm_info->tm_hour,
		tm_info->tm_min,
		tm_info->tm_sec);

	/* Rename current file */
	rename(handler->file_path, rotated_path);

	/* Open new file */
	handler->file_handle = fopen(handler->file_path, "a");
	if (handler->file_handle == NULL)
	{
		return 1;
	}

	return 0;
}

static int telemetry_log_should_rotate(telemetry_log_handler handler)
{
	long file_size;

	if (handler == NULL || handler->file_handle == NULL)
	{
		return 0;
	}

	/* Check size-based rotation */
	if (handler->rotation == TELEMETRY_LOG_ROTATION_SIZE && handler->max_file_size > 0)
	{
		fseek(handler->file_handle, 0, SEEK_END);
		file_size = ftell(handler->file_handle);

		if ((size_t)file_size >= handler->max_file_size)
		{
			return 1;
		}
	}

	/* TODO: Implement time-based rotation checks */

	return 0;
}
