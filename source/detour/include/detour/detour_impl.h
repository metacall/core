/*
*	Detour Library by Parra Studios
*	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library providing detours, function hooks and trampolines.
*
*/

#ifndef DETOUR_IMPL_H
#define DETOUR_IMPL_H 1

/* -- Headers -- */

#include <detour/detour_api.h>

#include <detour/detour_impl_handle.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct detour_impl_type;

/* -- Type Definitions -- */

typedef struct detour_impl_type * detour_impl;

/* -- Methods -- */

/**
*  @brief
*    Create detour implementation
*
*  @return
*    Returns pointer to detour implementation correct creation, null otherwise
*
*/
DETOUR_API detour_impl detour_impl_create(void);

/**
*  @brief
*    Load detour implementation @impl
*
*  @param[in] impl
*    Pointer to the detour implementation to be loaded
*
*  @param[in] path
*    Path where dependency is located
*
*  @param[in] name
*    Dependency name to be injected
*
*  @return
*    Returns zero on correct loading, distinct from zero otherwise
*
*/
DETOUR_API int detour_impl_load(detour_impl impl, const char * path, const char * name);

/**
*  @brief
*    Install detour implementation
*
*  @param[in] impl
*    Pointer to the detour hook implementation
*
*  @param[in] target
*    Pointer to the function to be intercepted
*
*  @param[in] hook
*    Function will be called instead of target
*
*  @return
*    Return pointer to the detour handle on success, null otherwise
*
*/
DETOUR_API detour_impl_handle detour_impl_install(detour_impl impl, void(**target)(void), void(*hook)(void));

/**
*  @brief
*    Uninstall detour implementation
*
*  @param[in] impl
*    Pointer to the detour hook implementation
*
*  @param[in] handle
*    Pointer to the detour hook handle
*
*  @return
*    Return zero on success, different from zero otherwise
*
*/
DETOUR_API int detour_impl_uninstall(detour_impl impl, detour_impl_handle handle);

/**
*  @brief
*    Unload detour implementation @impl
*
*  @param[in] impl
*    Pointer to the detour implementation to be unloaded
*
*  @return
*    Returns zero on correct unloading, distinct from zero otherwise
*
*/
DETOUR_API int detour_impl_unload(detour_impl impl);

/**
*  @brief
*    Destroy detour implementation
*
*  @param[in] impl
*    Pointer to the detour implementation to be destroyed
*
*  @return
*    Returns zero on correct destruction, distinct from zero otherwise
*
*/
DETOUR_API int detour_impl_destroy(detour_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* DETOUR_IMPL_H */
