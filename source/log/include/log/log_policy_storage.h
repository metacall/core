/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_POLICY_STORAGE_H
#define LOG_POLICY_STORAGE_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_policy.h>
#include <log/log_record.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

enum log_policy_storage_id
{
	LOG_POLICY_STORAGE_BATCH		= 0x00,
	LOG_POLICY_STORAGE_SEQUENTIAL	= 0x01,

	LOG_POLICY_STORAGE_SIZE
};

/* -- Forward Declarations -- */

struct log_policy_storage_impl_type;

/* -- Type Definitions -- */

typedef struct log_policy_storage_impl_type * log_policy_storage_impl;

typedef int (*log_policy_storage_append)(log_policy, const log_record);
typedef int (*log_policy_storage_flush)(log_policy);

/* -- Member Data -- */

struct log_policy_storage_impl_type
{
	log_policy_storage_append append;
	log_policy_storage_flush flush;
};

/* -- Methods -- */

LOG_API log_policy_interface log_policy_storage(const log_policy_id policy_storage_id);

LOG_API log_policy log_policy_storage_batch(size_t size);

LOG_API log_policy log_policy_storage_sequential(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_POLICY_STORAGE_H */
