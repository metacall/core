/*
 *	Configuration Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing json configuration format.
 *
 */

#ifndef RAPID_JSON_CONFIG_H
#define RAPID_JSON_CONFIG_H 1

/* -- Headers -- */

#include <rapid_json_config/rapid_json_config_api.h>

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
RAPID_JSON_CONFIG_API configuration_interface configuration_interface_instance_rapid_json(void);

#ifdef __cplusplus
}
#endif

#endif /* RAPID_JSON_CONFIG_H */
