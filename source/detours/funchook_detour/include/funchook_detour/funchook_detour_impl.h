/*
 *	Detour Library by Parra Studios
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef FUNCHOOK_DETOUR_IMPL_H
#define FUNCHOOK_DETOUR_IMPL_H 1

/* -- Headers -- */

#include <funchook_detour/funchook_detour_api.h>

#include <detour/detour_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Initialize FuncHook detour hook implementation
*
*  @return
*    Returns pointer to detour hook implementation on success, null pointer otherwise
*
*/
FUNCHOOK_DETOUR_API detour_impl_handle funchook_detour_impl_initialize(void);

/**
*  @brief
*    Install FuncHook detour implementation
*
*  @param[in] handle
*    Pointer to the detour hook implementation
*
*  @param[in] target
*    Pointer to the function to be intercepted
*
*  @param[in] hook
*    Function will be called instead of target
*
*  @return
*    Return zero on success, different from zero otherwise
*
*/
FUNCHOOK_DETOUR_API int funchook_detour_impl_install(detour_impl_handle handle, void (**target)(void), void (*hook)(void));

/**
*  @brief
*    Uninstall FuncHook detour implementation
*
*  @param[in] handle
*    Pointer to the detour hook implementation
*
*  @return
*    Return zero on success, different from zero otherwise
*
*/
FUNCHOOK_DETOUR_API int funchook_detour_impl_uninstall(detour_impl_handle handle);

/**
*  @brief
*    Destroy FuncHook detour implementation
*
*  @return
*    Returns zero on correct destruction, distinct from zero otherwise
*
*/
FUNCHOOK_DETOUR_API int funchook_detour_impl_destroy(detour_impl_handle handle);

#ifdef __cplusplus
}
#endif

#endif /* FUNCHOOK_DETOUR_IMPL_H */
