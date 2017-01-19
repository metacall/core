/*
 *	Configuration Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple configuration formats.
 *
 */

#ifndef CONFIGURATION_IMPL_RAPID_JSON_H
#define CONFIGURATION_IMPL_RAPID_JSON_H 1

/* -- Headers -- */

#include <configuration/configuration_api.h>

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
CONFIGURATION_API const char * configuration_impl_rapid_json_extension(void);

/**
*  @brief
*    Initialize RapidJSON implementation
*
*  @return
*    Returns zero on correct initialization, distinct from zero otherwise
*
*/
CONFIGURATION_API int configuration_impl_rapid_json_initialize(void);

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
CONFIGURATION_API configuration_impl configuration_impl_rapid_json_load(configuration config);

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
CONFIGURATION_API int configuration_impl_rapid_json_unload(configuration config);

/**
*  @brief
*    Destroy configuration object RapidJSON implementation
*
*  @return
*    Returns zero on correct destruction, distinct from zero otherwise
*
*/
CONFIGURATION_API int configuration_impl_rapid_json_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* CONFIGURATION_IMPL_RAPID_JSON_H */
