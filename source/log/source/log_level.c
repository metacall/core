/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_level.h>

#include <string.h>

/* -- Private Variables -- */

static const char * level_name_list[LOG_LEVEL_SIZE] =
{
	"Debug",
	"Information",
	"Warning",
	"Error",
	"Critical"
};

/* -- Methods -- */

const char * log_level_to_string(enum log_level_id level)
{
	return level_name_list[level];
}

enum log_level_id log_level_to_enum(const char * level, size_t length)
{
	size_t iterator, size = sizeof(level_name_list) / sizeof(level_name_list[0]);

	for (iterator = 0; iterator < size; ++iterator)
	{
		if (strncmp(level_name_list[iterator], level, length) == 0)
		{
			return iterator;
		}
	}

	return LOG_LEVEL_SIZE;
}
