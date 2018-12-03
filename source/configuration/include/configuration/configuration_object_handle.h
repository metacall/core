/*
 *	Configuration Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple configuration formats.
 *
 */

#ifndef CONFIGURATION_OBJECT_HANDLE_H
#define CONFIGURATION_OBJECT_HANDLE_H 1

/* -- Headers -- */

#include <configuration/configuration_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct configuration_type;

/* -- Type Definitions -- */

typedef struct configuration_type * configuration;

#ifdef __cplusplus
}
#endif

#endif /* CONFIGURATION_OBJECT_HANDLE_H */
