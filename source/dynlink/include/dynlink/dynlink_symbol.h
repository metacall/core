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

#ifndef DYNLINK_SYMBOL_H
#define DYNLINK_SYMBOL_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#include <dynlink/dynlink_type.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#define DYNLINK_SYMBOL_NAME_SIZE 0xFF

/* -- Type Definitions -- */

typedef char dynlink_symbol_name_man[DYNLINK_SYMBOL_NAME_SIZE];

/* -- Methods -- */

/**
*  @brief
*    Get convert a symbol to name mangled for cross-platform dynamic loading
*
*  @param[in] symbol_name
*    Reference to name of the of dynamically linked shared object symbol
*
*  @param[out] symbol_mangled
*    Reference to mangled name of the of dynamically linked shared object symbol
*
*  @return
*    Returns zero if @symbol_name was correctly mangled
*/
DYNLINK_API size_t dynlink_symbol_name_mangle(dynlink_symbol_name symbol_name, size_t symbol_name_length, dynlink_symbol_name_man symbol_mangled);

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_H */
