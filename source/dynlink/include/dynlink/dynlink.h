/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef DYNLINK_H
#define DYNLINK_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#include <dynlink/dynlink_flags.h>
#include <dynlink/dynlink_interface.h>
#include <dynlink/dynlink_symbol.h>
#include <dynlink/dynlink_type.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Get the library extension for specified platform
*
*  @return
*    A constant string pointer to the platform extension
*/
DYNLINK_API const char *dynlink_extension(void);

/**
*  @brief
*    Load a dynamically linked shared object
*
*  @param[in] path
*    Path where is located the shared object
*
*  @param[in] name
*    Name identifier of the shared object
*
*  @param[in] flags
*    Dynamic linking flags
*
*  @return
*    A handle to the dynamically linked shared object
*/
DYNLINK_API dynlink dynlink_load(dynlink_path path, dynlink_name name, dynlink_flags flags);

/**
*  @brief
*    Retreive the name of the dynamically linked shared object
*
*  @param[in] handle
*    Handle of dynamically linked shared object
*
*  @return
*    Reference to the name of the dynamically linked shared object
*/
DYNLINK_API dynlink_name dynlink_get_name(dynlink handle);

/**
*  @brief
*    Retreive the file name of the dynamically linked shared object handle
*
*  @param[in] handle
*    Handle of dynamically linked shared object
*
*  @return
*    Reference to the file name of the dynamically linked shared object
*/
DYNLINK_API dynlink_name dynlink_get_name_impl(dynlink handle);

/**
*  @brief
*    Retreive the linking flags of the dynamically linked shared object
*
*  @param[in] handle
*    Handle of dynamically linked shared object
*
*  @return
*    Linking flags of dynamically linked shared object
*/
DYNLINK_API dynlink_flags dynlink_get_flags(dynlink handle);

/**
*  @brief
*    Get a symbol address of dynamically linked shared object by name
*
*  @param[in] handle
*    Handle of dynamically linked shared object
*
*  @param[in] symbol_name
*    Name of the of dynamically linked shared object symbol
*
*  @param[out] symbol_address
*    Pointer to the address of the of dynamically linked shared object symbol
*
*  @return
*    Returns zero on correct dynamic linking, distinct from zero otherwise
*/
DYNLINK_API int dynlink_symbol(dynlink handle, dynlink_symbol_name symbol_name, dynlink_symbol_addr *symbol_address);

/**
*  @brief
*    Unloads a dynamically linked shared object by its handle
*
*  @param[in] handle
*    Handle of dynamically linked shared object
*/
DYNLINK_API void dynlink_unload(dynlink handle);

/**
*  @brief
*    Returns the path to a library loaded in the process itself
*
*  @param[in] name
*    Name of the library that will be searched for the path
*
*  @return
*    Returns a reference to a string that must be freed or NULL if no support or if the library could not be found
*/
DYNLINK_API char *dynlink_lib_path(dynlink_name name);

/**
*  @brief
*    Provide the module information
*
*  @return
*    Static string containing module information
*
*/
DYNLINK_API const char *dynlink_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_H */
