/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

#ifndef FUNHOOK_DETOUR_IMPL_H
#define FUNHOOK_DETOUR_IMPL_H 1

/* -- Headers -- */

#include <funhook_detour/funhook_detour_api.h>

#include <detour/detour_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Initialize FunHook detour hook implementation
*
*  @return
*    Returns pointer to detour hook implementation on success, null pointer otherwise
*
*/
FUNHOOK_DETOUR_API detour_impl_handle funhook_detour_impl_initialize(void);

/**
*  @brief
*    Install FunHook detour implementation
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
FUNHOOK_DETOUR_API int funhook_detour_impl_install(detour_impl_handle handle, void ** target, void * hook);

/**
*  @brief
*    Get trampoline from FunHook detour implementation
*
*  @param[in] handle
*    Pointer to the detour hook implementation
*
*  @return
*    Return pointer to trampoline already hooked
*
*/
FUNHOOK_DETOUR_API void * funhook_detour_impl_trampoline(detour_impl_handle handle);

/**
*  @brief
*    Uninstall FunHook detour implementation
*
*  @param[in] handle
*    Pointer to the detour hook implementation
*
*  @return
*    Return zero on success, different from zero otherwise
*
*/
FUNHOOK_DETOUR_API int funhook_detour_impl_uninstall(detour_impl_handle handle);

/**
*  @brief
*    Destroy FunHook detour implementation
*
*  @return
*    Returns zero on correct destruction, distinct from zero otherwise
*
*/
FUNHOOK_DETOUR_API int funhook_detour_impl_destroy(detour_impl_handle handle);

#ifdef __cplusplus
}
#endif

#endif /* FUNHOOK_DETOUR_IMPL_H */
