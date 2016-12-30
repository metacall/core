/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_POLICY_H
#define LOG_POLICY_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_aspect.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Forward Declarations -- */

struct log_aspect_type;

struct log_policy_type;

struct log_policy_ctor_type;

struct log_policy_interface_type;

/* -- Type Definitions -- */

typedef struct log_aspect_type * log_aspect;

typedef size_t log_policy_id;

typedef struct log_policy_type * log_policy;

typedef void * log_policy_impl;

typedef void * log_policy_data;

typedef void * log_policy_ctor;

typedef int (*log_policy_interface_create)(log_policy, const log_policy_ctor);

typedef int (*log_policy_interface_destroy)(log_policy);

typedef struct log_policy_interface_type * log_policy_interface;

typedef log_policy_interface (*log_policy_singleton)(void);

/* -- Member Data -- */

struct log_policy_interface_type
{
	log_policy_interface_create create;
	log_policy_impl impl;
	log_policy_interface_destroy destroy;
};

/* -- Methods -- */

LOG_API log_policy log_policy_create(enum log_aspect_id aspect_id, const log_policy_interface iface, const log_policy_ctor ctor);

LOG_API void log_policy_instantiate(log_policy policy, log_policy_data instance, const log_policy_id id);

LOG_API void log_policy_classify(log_policy policy, log_aspect aspect);

LOG_API log_aspect log_policy_aspect(log_policy policy);

LOG_API enum log_aspect_id log_policy_aspect_id(log_policy policy);

LOG_API log_policy_data log_policy_instance(log_policy policy);

LOG_API log_policy_interface log_policy_behavior(log_policy policy);

LOG_API log_policy_impl log_policy_derived(log_policy policy);

LOG_API int log_policy_destroy(log_policy policy);

#ifdef __cplusplus
}
#endif

#endif /* LOG_POLICY_H */
