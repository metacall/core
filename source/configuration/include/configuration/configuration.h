/*
 *	Configuration Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple configuration formats.
 *
 */

#ifndef CONFIGURATION_H
#define CONFIGURATION_H 1

/* -- Headers -- */

#include <configuration/configuration_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

/* -- Forward Declarations -- */

/* -- Type Definitions -- */

/* -- Methods -- */

/**
*  @brief
*    Provide the module information
*
*  @return
*    Static string containing module information
*
*/
CONFIGURATION_API const char * configuration_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* CONFIGURATION_H */
