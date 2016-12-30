/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_ASPECT_STORAGE_H
#define LOG_ASPECT_STORAGE_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_aspect.h>
#include <log/log_record.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct log_aspect_storage_impl_type;

/* -- Type Definitions -- */

typedef struct log_aspect_storage_impl_type * log_aspect_storage_impl;

typedef int (*log_aspect_storage_append)(log_aspect, const log_record);
typedef int (*log_aspect_storage_flush)(log_aspect);

/* -- Member Data -- */

struct log_aspect_storage_impl_type
{
	log_aspect_storage_append append;
	log_aspect_storage_flush flush;
};

/* -- Methods -- */

LOG_API log_aspect_interface log_aspect_storage_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_ASPECT_STORAGE_H */
