/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

#ifndef DETOUR_H
#define DETOUR_H 1

/* -- Headers -- */

#include <detour/detour_api.h>

#include <detour/detour_handle.h>

#include <plugin/plugin.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Type Definitions -- */

typedef plugin detour;

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
*    Plugin will be used for hooking
*
*  @return
*    Pointer to detour on correct initialization, null otherwise
*
*/
DETOUR_API detour detour_create(const char *name);

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
DETOUR_API const char *detour_name(detour d);

/**
*  @brief
*    Initialize the detour of a library from @path
*
*  @param[in] d
*    Reference to the detour
*
*  @param[in] path
*    String to the path or name of the library, in case of NULL, the current process will be used
*
*  @return
*    Pointer to the detour handle
*
*/
DETOUR_API detour_handle detour_load_file(detour d, const char *path);

/**
*  @brief
*    Initialize the detour of a library from @library dynlink handle
*
*  @param[in] d
*    Reference to the detour
*
*  @param[in] library
*    Pointer to the library already opened by dynlink
*
*  @return
*    Pointer to the detour handle
*
*/
DETOUR_API detour_handle detour_load_handle(detour d, dynlink library);

/**
*  @brief
*    Initialize the detour of a library from @address, this function pointer
*    must be a pointer to a function of the library that we want to hook
*
*  @param[in] d
*    Reference to the detour
*
*  @param[in] address
*    Pointer to a function of the library we want to hook
*
*  @return
*    Pointer to the detour handle
*
*/
DETOUR_API detour_handle detour_load_address(detour d, void (*address)(void));

/**
*  @brief
*    Iterate all symbols of the library already opened
*
*  @param[in] d
*    Reference to the detour
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
DETOUR_API int detour_enumerate(detour d, detour_handle handle, unsigned int *position, const char **name, void (***address)(void));

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
*  @param[in] function_name
*    Function name to be hooked, it must belong to the library
*
*  @param[in] function_addr
*    Function pointer to the function that will replace the original function from the library
*
*  @param[out] function_trampoline
*    Function pointer to the original function from the library that will be replaced
*
*  @return
*    Return zero if success, different from zero otherwise
*
*/
DETOUR_API int detour_replace(detour d, detour_handle handle, const char *function_name, void (*function_addr)(void), void (**function_trampoline)(void));

/**
*  @brief
*    Destroy detour handle previously loaded by detour_load_* functions
*
*  @param[in] d
*    Reference to the detour
*
*  @param[in] handle
*    Reference to the detour handle
*
*/
DETOUR_API void detour_unload(detour d, detour_handle handle);

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
DETOUR_API const char *detour_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* DETOUR_H */
