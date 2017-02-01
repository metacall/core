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

#include <dynlink/dynlink.h>

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
RAPID_JSON_CONFIG_API configuration_interface rapid_json_config_impl_interface_singleton(void);

DYNLINK_SYMBOL_EXPORT(rapid_json_config_impl_interface_singleton);

/**
*  @brief
*    Provide the module information
*
*  @return
*    Static string containing module information
*
*/
RAPID_JSON_CONFIG_API const char * rapid_json_config_print_info(void);

DYNLINK_SYMBOL_EXPORT(rapid_json_config_print_info);

#ifdef __cplusplus
}
#endif

#endif /* RAPID_JSON_CONFIG_H */
