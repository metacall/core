/*
 *	Configuration Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple configuration formats.
 *
 */

#ifndef CONFIGURATION_IMPL_H
#define CONFIGURATION_IMPL_H 1

/* -- Headers -- */

#include <configuration/configuration_api.h>

#include <configuration/configuration_object.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Initialize configuration object from @name @path and @parent
*
*  @param[in] name
*    Name of configuration object
*
*  @param[in] path
*    Path where is located the configuration object
*
*  @param[in] parent
*    Parent configuration inherited by new configuration
*
*  @return
*    Returns pointer to new configuration on correct configuration initialization, null otherwise
*
*/
CONFIGURATION_API int configuration_impl_initialize(configuration global);

/**
*  @brief
*    Destroy configuration object @config
*
*  @param[in] config
*    Pointer to configuration to be destroye
*
*/
CONFIGURATION_API void configuration_impl_destroy(configuration config);

#ifdef __cplusplus
}
#endif

#endif /* CONFIGURATION_IMPL_H */
