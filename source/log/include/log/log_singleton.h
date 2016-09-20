/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_SINGLETON_H
#define LOG_SINGLETON_H 1

/* -- Headers -- */

#include <log/log_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Forward Declarations -- */

struct log_impl_type;

struct log_singleton_type;

/* -- Type Definitions -- */

typedef struct log_impl_type * log_impl;

typedef struct log_singleton_type * log_singleton;

/* -- Member Data -- */

struct log_singleton_type
{
	log_impl *	map;
	size_t	size;
};

/* -- Protected Methods -- */

LOG_NO_EXPORT static int log_initialize(void);

LOG_NO_EXPORT static int log_destroy(void);

/* -- Methods -- */

LOG_API log_singleton log_instance(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_SINGLETON_H */
