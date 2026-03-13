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

#ifndef REFLECT_SCOPE_H
#define REFLECT_SCOPE_H 1

#include <reflect/reflect_api.h>

#include <reflect/reflect_function.h>
#include <reflect/reflect_type.h>

#ifdef __cplusplus
extern "C" {
#endif

struct scope_type;

typedef size_t scope_stack_ptr;

typedef struct scope_type *scope;

/**
*  @brief
*    Create a new scope with the given name
*
*  @param[in] name
*    Name identifier for the scope (e.g. "global_namespace").
*    The name string is copied internally; the caller does not need
*    to keep it alive after this call.
*
*  @return
*    Pointer to the newly created scope on success, NULL on failure
*/
REFLECT_API scope scope_create(const char *name);

/**
*  @brief
*    Get the number of objects registered in the scope
*
*  @param[in] sp
*    Pointer to the scope
*
*  @return
*    Number of objects (functions, classes, objects) in the scope
*/
REFLECT_API size_t scope_size(scope sp);

/**
*  @brief
*    Register a named object (function, class, or object) into the scope
*
*  @param[in] sp
*    Pointer to the scope
*  @param[in] key
*    Name under which the object will be registered
*  @param[in] obj
*    Value to register in the scope
*
*  @return
*    Zero on success, different from zero on failure
*/
REFLECT_API int scope_define(scope sp, const char *key, value obj);

/**
*  @brief
*    Generate a metadata representation of the scope and its contents
*
*  @param[in] sp
*    Pointer to the scope
*
*  @return
*    Value containing the metadata map, or NULL on failure
*/
REFLECT_API value scope_metadata(scope sp);

/**
*  @brief
*    Export the scope contents as a serializable value
*
*  @param[in] sp
*    Pointer to the scope
*
*  @return
*    Value containing the exported scope data, or NULL on failure
*/
REFLECT_API value scope_export(scope sp);

/**
*  @brief
*    Retrieve a registered object from the scope by name
*
*  @param[in] sp
*    Pointer to the scope
*  @param[in] key
*    Name of the object to retrieve
*
*  @return
*    Value associated with the key, or NULL if not found
*/
REFLECT_API value scope_get(scope sp, const char *key);

/**
*  @brief
*    Remove a registered object from the scope by name
*
*  @param[in] sp
*    Pointer to the scope
*  @param[in] key
*    Name of the object to undefine
*
*  @return
*    Value that was removed, or NULL if not found
*/
REFLECT_API value scope_undef(scope sp, const char *key);

/**
*  @brief
*    Append all objects from source scope into destination scope
*
*  @param[in] dest
*    Destination scope to append into
*  @param[in] src
*    Source scope whose objects will be copied
*
*  @return
*    Zero on success, different from zero on failure
*/
REFLECT_API int scope_append(scope dest, scope src);

/**
*  @brief
*    Check if any objects in source scope already exist in destination scope
*
*  @param[in] dest
*    Destination scope to check against
*  @param[in] src
*    Source scope to check for duplicates
*  @param[out] duplicated
*    Pointer to receive the name of the first duplicate found, if any
*
*  @return
*    Zero if a duplicate key is found, different from zero if the scopes
*    are disjoint or an error occurs
*/
REFLECT_API int scope_contains(scope dest, scope src, char **duplicated);

/**
*  @brief
*    Remove all objects that exist in source scope from destination scope
*
*  @param[in] dest
*    Destination scope to remove from
*  @param[in] src
*    Source scope whose objects will be removed from dest
*
*  @return
*    Zero on success, different from zero on failure
*/
REFLECT_API int scope_remove(scope dest, scope src);

/**
*  @brief
*    Get a pointer to the scope's internal return stack size
*
*  @param[in] sp
*    Pointer to the scope
*
*  @return
*    Pointer to the stack return size value
*/
REFLECT_API size_t *scope_stack_return(scope sp);

/**
*  @brief
*    Push a block of memory onto the scope's internal stack
*
*  @param[in] sp
*    Pointer to the scope
*  @param[in] bytes
*    Number of bytes to allocate on the stack
*
*  @return
*    Stack pointer offset to the allocated block
*/
REFLECT_API scope_stack_ptr scope_stack_push(scope sp, size_t bytes);

/**
*  @brief
*    Retrieve a pointer to data at the given stack position
*
*  @param[in] sp
*    Pointer to the scope
*  @param[in] stack_ptr
*    Stack pointer offset returned by scope_stack_push
*
*  @return
*    Pointer to the data at the given stack position, or NULL on failure
*/
REFLECT_API void *scope_stack_get(scope sp, scope_stack_ptr stack_ptr);

/**
*  @brief
*    Pop the last pushed block from the scope's internal stack
*
*  @param[in] sp
*    Pointer to the scope
*
*  @return
*    Zero on success, different from zero on failure
*/
REFLECT_API int scope_stack_pop(scope sp);

/**
*  @brief
*    Destroy the scope and free all associated resources
*
*  @param[in] sp
*    Pointer to the scope to destroy
*/
REFLECT_API void scope_destroy(scope sp);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_SCOPE_H */
