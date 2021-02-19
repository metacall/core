/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef DYNLINK_IMPL_H
#define DYNLINK_IMPL_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#include <dynlink/dynlink_type.h>
#include <dynlink/dynlink_impl_name.h>
#include <dynlink/dynlink_impl_type.h>
#include <dynlink/dynlink_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Methods -- */

/**
*  @brief
*    Dynamically linked shared object handle extension implementation
*
*  @return
*    A const string reference to the extension depending on the OS implementation
*/
DYNLINK_API const char * dynlink_impl_extension(void);

/**
*  @brief
*    Consturct the file name of dynamically linked shared object implementation
*
*  @param[in] handle
*    Handle of dynamically linked shared object
*
*  @param[out] name_impl
*    Pointer to the dynamically linked shared object handle
*/
DYNLINK_API void dynlink_impl_get_name(dynlink handle, dynlink_name_impl name_impl, size_t length);

/**
*  @brief
*    Load a dynamically linked shared object implementation
*
*  @param[in] name
*    Pointer to the dynamically linked shared object handle
*
*  @return
*    A pointer to the dynamically linked shared object implementation
*/
DYNLINK_API dynlink_impl dynlink_impl_load(dynlink handle);

/**
*  @brief
*    Get a symbol address of dynamically linked shared object by name implementation
*
*  @param[in] handle
*    Handle of dynamically linked shared object
*
*  @param[in] impl
*    Pointer to the dynamically linked shared object implementation
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
DYNLINK_API int dynlink_impl_symbol(dynlink handle, dynlink_impl impl, dynlink_symbol_name symbol_name, dynlink_symbol_addr * symbol_address);

/**
*  @brief
*    Unloads a dynamically linked shared object implementation by reference
*
*  @param[in] handle
*    Handle of dynamically linked shared object
*
*  @param[in] impl
*    Pointer to the dynamically linked shared object implementation
*/
DYNLINK_API void dynlink_impl_unload(dynlink handle, dynlink_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_IMPL_H */
