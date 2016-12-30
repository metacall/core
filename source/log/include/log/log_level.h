/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_LEVEL_H
#define LOG_LEVEL_H 1

#include <log/log_api.h>

#ifdef __cplusplus
extern "C" {
#endif

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

LOG_API const char * log_level_name(enum log_level_id level);

#ifdef __cplusplus
}
#endif

#endif /* LOG_LEVEL_H */
