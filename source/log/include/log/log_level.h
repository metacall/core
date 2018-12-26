/*
 *	Logger Library by Parra Studios
 *	A generic logger library providing application execution reports.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef LOG_LEVEL_H
#define LOG_LEVEL_H 1

#include <log/log_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Enumerations -- */

enum log_level_id
{
	LOG_LEVEL_DEBUG		= 0x00,
	LOG_LEVEL_INFO		= 0x01,
	LOG_LEVEL_WARNING	= 0x02,
	LOG_LEVEL_ERROR		= 0x03,
	LOG_LEVEL_CRITICAL	= 0x04,

	LOG_LEVEL_SIZE
};

/* -- Methods -- */

LOG_API const char * log_level_to_string(enum log_level_id level);

LOG_API enum log_level_id log_level_to_enum(const char * level, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* LOG_LEVEL_H */
