/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_level.h>

/* -- Methods -- */

const char * log_level_name(enum log_level_id level)
{
	static const char * level_name_list[LOG_LEVEL_SIZE] =
	{
		"Debug",
		"Information",
		"Warning",
		"Error",
		"Critical"
	};

	return level_name_list[level];
}
