/*
 *	Configuration Library by Parra Studios
 *	A cross-platform library for managing multiple configuration formats.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

#ifndef CONFIGURATION_OBJECT_H
#define CONFIGURATION_OBJECT_H 1

/* -- Headers -- */

#include <configuration/configuration_api.h>

#include <configuration/configuration_object_handle.h>

#include <adt/adt_vector.h>
#include <adt/adt_set.h>

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
*  @param[out] childs
*    Array of @config's childs
*
*  @param[inout] storage
*    Set containing already created configurations
*
*  @return
*    Returns zero on correct child initialization, distinct from zero otherwise
*
*/
CONFIGURATION_API int configuration_object_childs(configuration config, vector childs, set storage);

/**
*  @brief
*    Set value of configuration object @config
*
*  @param[in] config
*    Pointer to configuration object
*
*  @param[in] v
*    Pointer to configuration value representation
*
*/
CONFIGURATION_API void configuration_object_instantiate(configuration config, value v);

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
*    Retrieve the value of configuration object @config
*
*  @param[in] config
*    Pointer to configuration object
*
*  @return
*    Returns value representing configuration object @config
*
*/
CONFIGURATION_API value configuration_object_value(configuration config);

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
