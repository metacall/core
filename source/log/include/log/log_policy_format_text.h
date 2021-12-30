/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_POLICY_FORMAT_TEXT_H
#define LOG_POLICY_FORMAT_TEXT_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_policy.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Enumerations -- */

enum log_policy_format_text_id
{
	LOG_POLICY_FORMAT_TEXT_EMPTY = 0x00,
	LOG_POLICY_FORMAT_TEXT_NEWLINE = 0x01 << 0x00
};

/* -- Forward Declarations -- */

struct log_policy_format_text_ctor_type;

/* -- Type Definitions -- */

typedef struct log_policy_format_text_ctor_type *log_policy_format_text_ctor;

/* -- Member Data -- */

struct log_policy_format_text_ctor_type
{
	unsigned int flags;
};

/* -- Methods -- */

LOG_API log_policy_interface log_policy_format_text_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_POLICY_FORMAT_TEXT_H */
