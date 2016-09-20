/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_ASPECT_H
#define LOG_ASPECT_H 1

/* -- Headers -- */

#include <log/log_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Definitions -- */
enum log_aspect_id
{
	LOG_ASPECT_SCHEDULE	= 0x00,
	LOG_ASPECT_STORAGE	= 0x01,
	LOG_ASPECT_STREAM	= 0x02,

	LOG_ASPECT_SIZE
};

/* -- Forward Declarations -- */

struct log_policy_type;

struct log_aspect_type;

struct log_aspect_impl_type;

/* -- Type Definitions -- */

typedef size_t log_policy_id;

typedef struct log_policy_type * log_policy;

typedef void * log_aspect_impl;

typedef void * log_aspect_data;

typedef void * log_aspect_ctor;

typedef struct log_aspect_type * log_aspect;

typedef void * log_aspect_notify_data;

typedef struct log_aspect_interface_type * log_aspect_interface;

typedef int (*log_aspect_restrict_cb)(log_aspect, log_policy);

typedef int (*log_aspect_notify_cb)(log_aspect, log_policy, log_aspect_notify_data);

typedef log_aspect_data (*log_aspect_impl_create)(log_aspect, const log_aspect_ctor);

typedef int (*log_aspect_impl_destroy)(log_aspect);

typedef const log_aspect_interface (*log_aspect_singleton)(void);

/* -- Member Data -- */

struct log_aspect_interface_type
{
	log_aspect_impl_create create;
	log_aspect_impl impl;
	log_aspect_impl_destroy destroy;
};

/* -- Methods -- */

LOG_API log_aspect log_aspect_create(const log_aspect_interface iface, const log_aspect_ctor ctor);

LOG_API int log_aspect_reserve(log_aspect aspect, size_t size);

LOG_API void log_aspect_restrict(log_aspect aspect, log_aspect_restrict_cb restrict_cb);

LOG_API log_aspect_data log_aspect_instance(log_aspect aspect);

LOG_API int log_aspect_attach(log_aspect aspect, log_policy policy);

LOG_API int log_aspect_notify_all(log_aspect aspect, log_aspect_notify_cb notify_cb, log_aspect_notify_data notify_data);

LOG_API int log_aspect_detach(log_aspect aspect, log_policy policy);

LOG_API int log_aspect_destroy(log_aspect aspect);

#ifdef __cplusplus
}
#endif

#endif /* LOG_ASPECT_H */
