/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_POLICY_FORMAT_H
#define LOG_POLICY_FORMAT_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_policy.h>
#include <log/log_policy_format_binary.h>
#include <log/log_policy_format_custom.h>
#include <log/log_policy_format_text.h>

#include <log/log_record.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Enumerations -- */

enum log_policy_format_id
{
	LOG_POLICY_FORMAT_BINARY = 0x00,
	LOG_POLICY_FORMAT_TEXT = 0x01,
	LOG_POLICY_FORMAT_CUSTOM = 0x02,

	LOG_POLICY_FORMAT_SIZE
};

/* -- Forward Declarations -- */

struct log_policy_format_impl_type;

/* -- Type Definitions -- */

typedef struct log_policy_format_impl_type *log_policy_format_impl;

typedef size_t (*log_policy_format_size)(log_policy, const log_record);
typedef size_t (*log_policy_format_serialize)(log_policy, const log_record, void *, const size_t);
typedef size_t (*log_policy_format_deserialize)(log_policy, log_record, const void *, const size_t);

/* -- Member Data -- */

struct log_policy_format_impl_type
{
	log_policy_format_size size;
	log_policy_format_serialize serialize;
	log_policy_format_deserialize deserialize;
};

/* -- Methods -- */

LOG_API log_policy_interface log_policy_format(const log_policy_id policy_format_id);

LOG_API log_policy log_policy_format_binary(void);

LOG_API log_policy log_policy_format_text(void);

LOG_API log_policy log_policy_format_text_flags(unsigned int flags);

LOG_API log_policy log_policy_format_custom(void *context, log_policy_format_custom_size_ptr format_size, log_policy_format_custom_serialize_ptr format_serialize, log_policy_format_custom_deserialize_ptr format_deserialize);

#ifdef __cplusplus
}
#endif

#endif /* LOG_POLICY_FORMAT_H */
