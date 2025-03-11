/*
 *	MetaCall Log by Parra Studios
 *	Example of advanced logging in MetaCall.
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

/* -- Headers -- */

#include <metacall/metacall.h>

#include <cstdio>
#include <cstring>

/* --  -- */

static const char format[] = "%.19s #%d %s:%d %s @%s ";

/* -- Private Methods -- */

static size_t format_size(void *context, const char *time, uint64_t id, size_t line, const char *func, const char *file, const char *level, const char *message, metacall_log_custom_va_list args)
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

	return snprintf(NULL, 0, format, time, (int)id, file, (int)line, func, level) + length + 1;
}

static size_t format_serialize(void *context, void *buffer, const size_t size, const char *time, uint64_t id, size_t line, const char *func, const char *file, const char *level, const char *message, metacall_log_custom_va_list args)
{
	size_t length = snprintf((char *)buffer, size, format, time, (int)id, file, (int)line, func, level);
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

static size_t format_deserialize(void *context, const void *buffer, const size_t size, const char *time, uint64_t id, size_t line, const char *func, const char *file, const char *level, const char *message, metacall_log_custom_va_list args)
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

static int stream_flush(void *context)
{
	(void)context;

	fflush(stdout);

	return 0;
}

static int stream_write(void *context, const char *buffer, const size_t size)
{
	(void)size;

	printf("%s: %s\n", (const char *)context, buffer);

	return 0;
}

/* -- Methods -- */

int main(int, char *[])
{
	static const char context[] = "custom log";

	struct metacall_log_custom_type custom_log = {
		(void *)context,
		&format_size,
		&format_serialize,
		&format_deserialize,
		&stream_write,
		&stream_flush
	};

	if (metacall_log(METACALL_LOG_CUSTOM, (void *)&custom_log) != 0)
	{
		return 1;
	}

	if (metacall_initialize() != 0)
	{
		return 1;
	}

	/* Here you can load some scripts */

	metacall_destroy();

	return 0;
}
