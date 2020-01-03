/*
 *	Configuration Library by Parra Studios
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple configuration formats.
 *
 */

#ifndef CONFIGURATION_H
#define CONFIGURATION_H 1

/* -- Headers -- */

#include <configuration/configuration_api.h>

#include <configuration/configuration_object_handle.h>

#include <reflect/reflect_value_type.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#define CONFIGURATION_GLOBAL_SCOPE "global"

/* -- Methods -- */

/**
*  @brief
*    Initialize configuration from root @path with reader @name
*
*  @param[in] reader
*    Reader will be used to parse configurations
*
*  @param[in] path
*    Path where is located the root configuration
*
*  @param[in] allocator
*    Pointer to the allocator will be used to deserialize the configuration
*
*  @return
*    Returns zero on correct configuration initialization, distinct from zero otherwise
*
*/
CONFIGURATION_API int configuration_initialize(const char * reader, const char * path, void * allocator);

/**
*  @brief
*    Create configuration from @path with name @name
*
*  @param[in] scope
*    Scope of the configuration
*
*  @param[in] path
*    Path where is located the child configuration
*
*  @param[in] parent
*    Scope of the parent configuration if any
*
*  @param[in] allocator
*    Pointer to the allocator will be used to deserialize the configuration
*
*  @return
*    Returns object containing all configuration data
*
*/
CONFIGURATION_API configuration configuration_create(const char * scope, const char * path, const char * parent, void * allocator);

/**
*  @brief
*    Get configuration from @scope
*
*  @param[in] scope
*    Scope of the configuration
*
*  @return
*    Returns object containing all configuration data
*
*/
CONFIGURATION_API configuration configuration_scope(const char * scope);

/**
*  @brief
*    Get value from @key inside a configuration @config
*
*  @param[in] config
*    Configuration
*
*  @param[in] key
*    Scope of the configuration
*
*  @return
*    Returns value containing data related to @key or null if value or type @id are invalid
*
*/
CONFIGURATION_API value configuration_value(configuration config, const char * key);

/**
*  @brief
*    Delete a configuration @config from the system
*
*  @param[in] config
*    Configuration
*
*  @return
*    Returns zero on correct configuration clearing, distinct from zero otherwise
*
*/
CONFIGURATION_API int configuration_clear(configuration config);

/**
*  @brief
*    Provide the module information
*
*/
CONFIGURATION_API void configuration_destroy(void);

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
