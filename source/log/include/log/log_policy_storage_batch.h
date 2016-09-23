/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_POLICY_STORAGE_BATCH_H
#define LOG_POLICY_STORAGE_BATCH_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_policy.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct log_policy_storage_batch_ctor_type;

/* -- Type Definitions -- */

typedef struct log_policy_storage_batch_ctor_type * log_policy_storage_batch_ctor;

/* -- Member Data -- */

struct log_policy_storage_batch_ctor_type
{
	size_t size;
};

/* -- Methods -- */

LOG_API log_policy_interface log_policy_storage_batch_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_POLICY_STORAGE_BATCH_H */
