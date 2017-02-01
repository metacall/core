/*
 *	Configuration Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing json configuration format.
 *
 */

#ifndef RAPID_JSON_CONFIG_IMPL_H
#define RAPID_JSON_CONFIG_IMPL_H 1

/* -- Headers -- */

#include <rapid_json_config/rapid_json_config_api.h>

#include <configuration/configuration_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Retrieve extension supported by RapidJSON implementation
*
*  @return
*    Returns constant string representing configuration extension
*
*/
RAPID_JSON_CONFIG_API const char * rapid_json_config_impl_extension(void);

/**
*  @brief
*    Initialize RapidJSON implementation
*
*  @return
*    Returns zero on correct initialization, distinct from zero otherwise
*
*/
RAPID_JSON_CONFIG_API int rapid_json_config_impl_initialize(void);

/**
*  @brief
*    Load configuration object RapidJSON implementation @config
*
*  @param[in] config
*    Pointer to the config object to be loaded
*
*  @return
*    Returns pointer to implementation on correct loading, null otherwise
*
*/
RAPID_JSON_CONFIG_API configuration_impl rapid_json_config_impl_load(configuration config);

/**
*  @brief
*    Unload configuration object RapidJSON implementation @config
*
*  @param[in] config
*    Pointer to the config object to be unloaded
*
*  @return
*    Returns zero on correct unloading, distinct from zero otherwise
*
*/
RAPID_JSON_CONFIG_API int rapid_json_config_impl_unload(configuration config);

/**
*  @brief
*    Destroy configuration object RapidJSON implementation
*
*  @return
*    Returns zero on correct destruction, distinct from zero otherwise
*
*/
RAPID_JSON_CONFIG_API int rapid_json_config_impl_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* RAPID_JSON_CONFIG_IMPL_H */
