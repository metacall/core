/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef REFLECT_TYPE_H
#define REFLECT_TYPE_H 1

#include <reflect/reflect_api.h>

#include <reflect/reflect_type_id.h>
#include <reflect/reflect_value.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

struct type_type;

typedef struct type_type *type;

typedef void *type_impl;

typedef int (*type_impl_interface_create)(type, type_impl);

typedef void (*type_impl_interface_destroy)(type, type_impl);

typedef struct type_interface_type
{
	type_impl_interface_create create;
	type_impl_interface_destroy destroy;

} * type_interface;

typedef type_interface (*type_impl_interface_singleton)(void);

/**
*  @brief
*    Create a new type representation
*
*  @param[in] id
*    Numeric type identifier (e.g. TYPE_INT, TYPE_STRING)
*  @param[in] name
*    Human-readable name of the type (e.g. "Integer", "String")
*  @param[in] impl
*    Pointer to the language-specific type implementation
*  @param[in] singleton
*    Singleton accessor for the type interface (create, destroy)
*
*  @return
*    Pointer to the newly created type on success, NULL on failure
*/
REFLECT_API type type_create(type_id id, const char *name, type_impl impl, type_impl_interface_singleton singleton);

/**
*  @brief
*    Get the numeric identifier of a type
*
*  @param[in] t
*    Pointer to the type
*
*  @return
*    Type identifier (e.g. TYPE_INT, TYPE_STRING)
*/
REFLECT_API type_id type_index(type t);

/**
*  @brief
*    Get the human-readable name of a type
*
*  @param[in] t
*    Pointer to the type
*
*  @return
*    String containing the type name
*/
REFLECT_API const char *type_name(type t);

/**
*  @brief
*    Get the language-specific derived implementation of a type
*
*  @param[in] t
*    Pointer to the type
*
*  @return
*    Opaque pointer to the derived type implementation
*/
REFLECT_API type_impl type_derived(type t);

/* TODO: Subtyping (for handling typed containers like arrays, maps or templates) */
/* REFLECT_API vector type_subtype(type t); */

/**
*  @brief
*    Generate a metadata representation of the type
*
*  @param[in] t
*    Pointer to the type
*
*  @return
*    Value containing the metadata map, or NULL on failure
*/
REFLECT_API value type_metadata(type t);

/**
*  @brief
*    Destroy a type and free all associated resources
*
*  @param[in] t
*    Pointer to the type to destroy
*/
REFLECT_API void type_destroy(type t);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_TYPE_H */
