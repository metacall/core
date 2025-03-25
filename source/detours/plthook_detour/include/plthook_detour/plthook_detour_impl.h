/*
 *	Detour Library by Parra Studios
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef PLTHOOK_DETOUR_IMPL_H
#define PLTHOOK_DETOUR_IMPL_H 1

/* -- Headers -- */

#include <plthook_detour/plthook_detour_api.h>

#include <detour/detour_interface.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Initialize PLTHook detour hook implementation by file name
*
*  @param[out] handle
*    When success, it returns the pointer to the detour implementation, null otherwise
*
*  @param[in] path
*    String containing the path or name to the dynamic library to be opened
*
*  @return
*    Returns zero on success, different from zero otherwise
*
*/
PLTHOOK_DETOUR_API int plthook_detour_impl_initialize_file(detour_impl_handle *handle, const char *path);

/**
*  @brief
*    Initialize PLTHook detour hook implementation by already l
*
*  @param[out] handle
*    When success, it returns the pointer to the detour implementation, null otherwiseoaded dynamic library handle
*
*  @param[in] library
*    Pointer to the dynlink handle of the library
*
*  @return
*    Returns zero on success, different from zero otherwise
*
*/
PLTHOOK_DETOUR_API int plthook_detour_impl_initialize_handle(detour_impl_handle *handle, dynlink library);

/**
*  @brief
*    Initialize PLTHook detour hook implementation by a functio
*
*  @param[out] handle
*    When success, it returns the pointer to the detour implementation, null otherwisen pointer of a function belonging to a library
*
*  @param[in] address
*    Function pointer of a function belonging to the library to be hooked
*
*  @return
*    Returns zero on success, different from zero otherwise
*
*/
PLTHOOK_DETOUR_API int plthook_detour_impl_initialize_address(detour_impl_handle *handle, void (*address)(void));

/**
*  @brief
*    Iterate all symbols of the library already opened
*
*  @param[in] handle
*    Pointer to the detour hook implementation
*
*  @param[out] position
*    Pointer to the current index of the enumeration
*
*  @param[out] name
*    Pointer to the function name in string form
*
*  @param[out] address
*    Pointer to the pointer of the function pointer of the function to be hooked
*
*  @return
*    Return zero on success, different from zero otherwise
*
*/
PLTHOOK_DETOUR_API int plthook_detour_impl_enumerate(detour_impl_handle handle, unsigned int *position, const char **name, void ***address);

/**
*  @brief
*    Replace function from a library already opened by name, returns the old function pointer
*
*  @param[in] handle
*    Pointer to the detour hook implementation
*
*  @param[in] function_name
*    String containing the function name to be replaced
*
*  @param[in] function_addr
*    Function pointer that will be used to replace the original one
*
*  @param[out] function_old_addr
*    Function pointer to the original function that has been replaced
*
*  @return
*    Return zero on success, different from zero otherwise
*
*/
PLTHOOK_DETOUR_API int plthook_detour_impl_replace(detour_impl_handle handle, const char *function_name, void (*function_addr)(void), void **function_old_addr);

/**
*  @brief
*    Error handling PLTHook detour implementation
*
*  @return
*    Returns string containing the information of the error
*
*/
PLTHOOK_DETOUR_API const char *plthook_detour_impl_error(detour_impl_handle handle);

/**
*  @brief
*    Destroy PLTHook detour implementation
*
*/
PLTHOOK_DETOUR_API void plthook_detour_impl_destroy(detour_impl_handle handle);

#ifdef __cplusplus
}
#endif

#endif /* PLTHOOK_DETOUR_IMPL_H */
