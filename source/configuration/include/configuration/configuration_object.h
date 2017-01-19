/*
 *	Configuration Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple configuration formats.
 *
 */

#ifndef CONFIGURATION_OBJECT_H
#define CONFIGURATION_OBJECT_H 1

/* -- Headers -- */

#include <configuration/configuration_api.h>

#include <configuration/configuration_object_handle.h>
#include <configuration/configuration_impl_handle.h>

#include <adt/adt_vector.h>

#include <reflect/reflect_value_type.h>

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
CONFIGURATION_API configuration configuration_object_initialize(const char * name, const char * path, configuration parent);

/**
*  @brief
*    Generate all childs from parent @config
*
*  @param[in] config
*    Pointer to configuration object
*
*  @param[in] childs
*    Array of @config's childs
*
*  @return
*    Returns zero on correct child initialization, distinct from zero otherwise
*
*/
CONFIGURATION_API int configuration_object_childs(configuration config, vector childs);

/**
*  @brief
*    Set configuration object implementation to configuration object @config
*
*  @param[in] config
*    Pointer to configuration object
*
*  @param[in] path
*    Pointer to configuration object implementation
*
*/
CONFIGURATION_API void configuration_object_instantiate(configuration config, configuration_impl impl);

/**
*  @brief
*    Retrieve the name of configuration object @config
*
*  @param[in] config
*    Pointer to configuration object
*
*  @return
*    Returns name of configuration object @config
*
*/
CONFIGURATION_API const char * configuration_object_name(configuration config);

/**
*  @brief
*    Retrieve the path of configuration object @config
*
*  @param[in] config
*    Pointer to configuration object
*
*  @return
*    Returns path of configuration object @config
*
*/
CONFIGURATION_API const char * configuration_object_path(configuration config);

/**
*  @brief
*    Retrieve the parent configuration object of configuration object @config
*
*  @param[in] config
*    Pointer to configuration object
*
*  @return
*    Returns parent of configuration object @config
*
*/
CONFIGURATION_API configuration configuration_object_parent(configuration config);

/**
*  @brief
*    Retrieve the source of configuration object @config
*
*  @param[in] config
*    Pointer to configuration object
*
*  @return
*    Returns source of configuration object @config
*
*/
CONFIGURATION_API const char * configuration_object_source(configuration config);

/**
*  @brief
*    Retrieve the implementation of configuration object @config
*
*  @param[in] config
*    Pointer to configuration object
*
*  @return
*    Returns implementation reference of configuration object @config
*
*/
CONFIGURATION_API configuration_impl configuration_object_impl(configuration config);

/**
*  @brief
*    Set a value @v by key @key into configuration @config
*
*  @param[in] config
*    Pointer to configuration object to be modified
*
*  @param[in] key
*    Key used to index the value @v
*
*  @param[in] value
*    Value to be inserted
*
*  @return
*    Returns zero on correct configuration set, distinct from zero otherwise
*
*/
CONFIGURATION_API int configuration_object_set(configuration config, const char * key, value v);

/**
*  @brief
*    Get a value by key @key from configuration @config
*
*  @param[in] config
*    Pointer to configuration object to be accessed
*
*  @param[in] key
*    Key used to retrieve by index the value @v
*
*  @return
*    Returns pointer to value if exists, null otherwise
*
*/
CONFIGURATION_API value configuration_object_get(configuration config, const char * key);

/**
*  @brief
*    Destroy configuration object @config
*
*  @param[in] config
*    Pointer to configuration to be destroyed
*
*/
CONFIGURATION_API void configuration_object_destroy(configuration config);

#ifdef __cplusplus
}
#endif

#endif /* CONFIGURATION_OBJECT_H */
