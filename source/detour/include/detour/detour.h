/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

#ifndef DETOUR_H
#define DETOUR_H 1

/* -- Headers -- */

#include <detour/detour_api.h>

#include <memory/memory.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct detour_type;

struct detour_handle_type;

/* -- Type Definitions -- */

typedef struct detour_type * detour;

typedef struct detour_handle_type * detour_handle;

/**
*  @brief
*    Initialize detour module
*
*  @return
*    Return zero correct initialization, distinct from zero otherwise
*
*/
DETOUR_API int detour_initialize(void);

/**
*  @brief
*    Create detour by @name
*
*  @param[in] name
*    Plugin will be used to detourize and detourize
*
*  @return
*    Pointer to detour on correct initialization, null otherwise
*
*/
DETOUR_API detour detour_create(const char * name);

/**
*  @brief
*    Get name of detour
*
*  @param[in] d
*    Reference to the detour
*
*  @return
*    Static const string with detour name
*
*/
DETOUR_API const char * detour_name(detour d);

/**
*  @brief
*    Get trampoline of the detour
*
*  @param[in] handle
*    Reference to the detour handle
*
*  @return
*    Pointer to the trampoline function
*
*/
DETOUR_API void (*detour_trampoline(detour_handle handle))(void);

/**
*  @brief
*    Install detour from @target to @hook
*
*  @param[in] d
*    Reference to the detour
*
*  @param[in] target
*    Reference to the function to be hooked
*
*  @param[in] hook
*    Reference to the function will be called instead of @target
*
*  @return
*    Pointer to the detour handle
*
*/
DETOUR_API detour_handle detour_install(detour d, void(*target)(void), void(*hook)(void));

/**
*  @brief
*    Install detour from @target to @hook
*
*  @param[in] d
*    Reference to the detour
*
*  @param[in] handle
*    Reference to the detour handle
*
*  @return
*    Return zero if success, different from zero otherwise
*
*/
DETOUR_API int detour_uninstall(detour d, detour_handle handle);

/**
*  @brief
*    Destroy detour by handle @d
*
*  @param[in] d
*    Reference to the detour
*
*  @return
*    Return zero on correct destruction, different from zero otherwise
*
*/
DETOUR_API int detour_clear(detour d);

/**
*  @brief
*    Destroy all detours
*
*/
DETOUR_API void detour_destroy(void);

/**
*  @brief
*    Provide the module information
*
*  @return
*    Static string containing module information
*
*/
DETOUR_API const char * detour_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* DETOUR_H */
