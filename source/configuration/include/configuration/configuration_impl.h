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
*    Retrieve extension supported by the configuration implementation
*
*  @return
*    Returns constant string representing configuration extension
*
*/
CONFIGURATION_API const char * configuration_impl_extension(void);

/**
*  @brief
*    Initialize configuration object implementation
*
*  @param[in] name
*    Dependency name to be injected
*
*  @return
*    Returns zero on correct initialization, distinct from zero otherwise
*
*/
CONFIGURATION_API int configuration_impl_initialize(const char * name);

/**
*  @brief
*    Load configuration object implementation @config
*
*  @param[in] config
*    Pointer to the config object to be loaded
*
*  @param[in] allocator
*    Pointer to the allocator will be used to deserialize the configuration
*
*  @return
*    Returns zero on correct loading, distinct from zero otherwise
*
*/
CONFIGURATION_API int configuration_impl_load(configuration config, void * allocator);

/**
*  @brief
*    Destroy configuration object implementation
*
*  @return
*    Returns zero on correct destruction, distinct from zero otherwise
*
*/
CONFIGURATION_API int configuration_impl_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* CONFIGURATION_IMPL_H */
