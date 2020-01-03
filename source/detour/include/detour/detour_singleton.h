/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

#ifndef DETOUR_SINGLETON_H
#define DETOUR_SINGLETON_H 1

/* -- Headers -- */

#include <detour/detour_api.h>

#include <detour/detour.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct detour_singleton_type;

/* -- Type Definitions -- */

typedef struct detour_singleton_type * detour_singleton;

/* -- Methods -- */

/**
*  @brief
*    Initialize detour singleton
*
*  @return
*    Returns zero on correct detour singleton initialization, distinct from zero otherwise
*
*/
DETOUR_API int detour_singleton_initialize();

/**
*  @brief
*    Register detour into detours map
*
*  @param[in] d
*    Pointer to detour
*
*  @return
*    Returns zero on correct detour singleton insertion, distinct from zero otherwise
*
*/
DETOUR_API int detour_singleton_register(detour d);

/**
*  @brief
*    Retrieve detour from detours map by @name
*
*  @param[in] name
*    Index which references the detour to be retrieved
*
*  @return
*    Returns pointer to detour if exists, null otherwise
*
*/
DETOUR_API detour detour_singleton_get(const char * name);

/**
*  @brief
*    Retrieve detour library path where detours are located
*
*  @return
*    Returns constant string representing detours library path
*
*/
DETOUR_API const char * detour_singleton_path(void);

/**
*  @brief
*    Remove detour from detours map
*
*  @param[in] d
*    Pointer to detour
*
*  @return
*    Returns zero on correct detour singleton removing, distinct from zero otherwise
*
*/
DETOUR_API int detour_singleton_clear(detour d);

/**
*  @brief
*    Destroy detour singleton
*
*/
DETOUR_API void detour_singleton_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* DETOUR_SINGLETON_H */
