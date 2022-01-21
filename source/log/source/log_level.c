/*
 *	Logger Library by Parra Studios
 *	A generic logger library providing application execution reports.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <log/log_level.h>

#include <string.h>

/* -- Private Variables -- */

static const char *level_name_list[LOG_LEVEL_SIZE] = {
	"Debug",
	"Information",
	"Warning",
	"Error",
	"Critical"
};

/* -- Methods -- */

const char *log_level_to_string(enum log_level_id level)
{
	return level_name_list[level];
}

enum log_level_id log_level_to_enum(const char *level, size_t length)
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
