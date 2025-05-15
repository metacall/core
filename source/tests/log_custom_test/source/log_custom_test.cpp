/*
 *	Logger Library by Parra Studios
 *	A generic logger library providing application execution reports.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

#include <gtest/gtest.h>

#include <format/format.h>
#include <log/log.h>
#include <log/log_handle.h>
#include <log/log_level.h>

static const char format[] = "%.19s #%" PRIu64 " %s:%" PRIuS " %s @%s ";

class log_custom_test : public testing::Test
{
public:
};

size_t format_size(void *context, const char *time, uint64_t id, size_t line, const char *func, const char *file, const char *level, const char *message, log_policy_format_custom_va_list args)
{
	size_t length = 0;

	(void)context;

	if (args != NULL)
	{
		va_list va;

		va_copy(va, args->va);

		length = vsnprintf(NULL, 0, message, va);

		va_end(va);
	}
	else
	{
		length = strlen(message);
	}

	return snprintf(NULL, 0, format, time, id, file, line, func, level) + length + 1;
}

size_t format_serialize(void *context, void *buffer, const size_t size, const char *time, uint64_t id, size_t line, const char *func, const char *file, const char *level, const char *message, log_policy_format_custom_va_list args)
{
	size_t length = snprintf((char *)buffer, size, format, time, id, file, line, func, level);
	char *body = &(((char *)buffer)[length]);

	(void)context;

	if (args != NULL)
	{
		va_list va;

		va_copy(va, args->va);

		length += vsnprintf(body, size - length, message, va);

		va_end(va);
	}
	else
	{
		length += snprintf(body, size - length, "%s", message);
	}

	return length + 1;
}

size_t format_deserialize(void *context, const void *buffer, const size_t size, const char *time, uint64_t id, size_t line, const char *func, const char *file, const char *level, const char *message, log_policy_format_custom_va_list args)
{
	/* TODO */
	(void)context;
	(void)buffer;
	(void)time;
	(void)id;
	(void)line;
	(void)func;
	(void)file;
	(void)level;
	(void)message;
	(void)args;

	return size;
}

int stream_flush(void *context)
{
	(void)context;

	fflush(stdout);

	return 0;
}

int stream_write(void *context, const char *buffer, const size_t size)
{
	(void)context;
	(void)size;

	printf("%s\n", buffer);

	return 0;
}

TEST_F(log_custom_test, DefaultConstructor)
{
	const char name[] = "custom_log";

	/* Create logs */
	EXPECT_EQ((int)0, (int)log_create(name));

	/* Set policies */
	EXPECT_EQ((int)0, (int)log_configure(name,
						  log_policy_format_custom(NULL, &format_size, &format_serialize, &format_deserialize),
						  log_policy_schedule_sync(),
						  log_policy_storage_sequential(),
						  log_policy_stream_custom(NULL, &stream_write, &stream_flush)));

	/* Write simple logs */
	EXPECT_EQ((int)0, (int)log_write(name, LOG_LEVEL_INFO, "hello world"));

	/* Write varidic log */
	EXPECT_EQ((int)0, (int)log_write(name, LOG_LEVEL_INFO, "hello world from log %d", 20));

	/* Clear log */
	EXPECT_EQ((int)0, (int)log_clear(name));
}
