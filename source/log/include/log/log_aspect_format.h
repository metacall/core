/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_ASPECT_FORMAT_H
#define LOG_ASPECT_FORMAT_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_aspect.h>
#include <log/log_record.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct log_aspect_format_impl_type;

/* -- Type Definitions -- */

typedef struct log_aspect_format_impl_type * log_aspect_format_impl;

typedef size_t (*log_aspect_format_size)(log_aspect, log_record);
typedef int (*log_aspect_format_serialize)(log_aspect, log_record, void *, const size_t);
typedef int (*log_aspect_format_deserialize)(log_aspect, log_record, const void *, const size_t);

/* -- Member Data -- */

struct log_aspect_format_impl_type
{
	log_aspect_format_size size;
	log_aspect_format_serialize serialize;
	log_aspect_format_deserialize deserialize;
};

/* -- Methods -- */

LOG_API log_aspect_interface log_aspect_format_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_ASPECT_FORMAT_H */
