/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_INTERFACE_H
#define LOG_INTERFACE_H 1

/* -- Headers -- */

#include <log/log_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct log_policy_type;

struct log_interface_type;

/* -- Type Definitions -- */

typedef struct log_policy_type * log_policy;

typedef int (*log_interface_create)(void);

typedef int (*log_interface_destroy)(void);

typedef struct log_interface_type * log_interface;

/* -- Member Data -- */

struct log_interface_type
{
	log_interface_create create;



	log_interface_destroy destroy;
};

/* -- Methods -- */

LOG_API void log_(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_INTERFACE_H */
