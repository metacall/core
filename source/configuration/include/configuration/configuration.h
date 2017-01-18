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

#include <configuration/configuration_object.h>

#include <reflect/reflect_value_type.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Initialize configuration from root @path
*
*  @param[in] path
*    Path where is located the root configuration
*
*  @return
*    Returns zero on correct configuration initialization, distinct from zero otherwise
*
*/
CONFIGURATION_API int configuration_initialize(const char * path);

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
CONFIGURATION_API value configuration_value(configuration config, const char * key, type_id id);

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
