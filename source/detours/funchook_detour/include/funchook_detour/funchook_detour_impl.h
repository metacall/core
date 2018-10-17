/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
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
FUNCHOOK_DETOUR_API detour_impl_handle funchook_detour_impl_initialize(detour_host host);

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
FUNCHOOK_DETOUR_API int funchook_detour_impl_install(detour_impl_handle handle, void(**target)(void), void(*hook)(void));

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
