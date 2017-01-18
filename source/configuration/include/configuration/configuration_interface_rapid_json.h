/*
 *	Configuration Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple configuration formats.
 *
 */

#ifndef CONFIGURATION_INTERFACE_RAPID_JSON_H
#define CONFIGURATION_INTERFACE_RAPID_JSON_H 1

/* -- Headers -- */

#include <configuration/configuration_api.h>

#include <configuration/configuration_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Instance of interface implementation
*
*  @return
*    Returns ointer to interface to be used by implementation
*
*/
CONFIGURATION_API configuration_interface configuration_interface_instance_rapid_json(void);

#ifdef __cplusplus
}
#endif

#endif /* CONFIGURATION_INTERFACE_RAPID_JSON_H */
