/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

#ifndef FUNHOOK_DETOUR_H
#define FUNHOOK_DETOUR_H 1

/* -- Headers -- */

#include <funhook_detour/funhook_detour_api.h>

#include <detour/detour_interface.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Instance of interface implementation
*
*  @return
*    Returns pointer to interface to be used by implementation
*
*/
FUNHOOK_DETOUR_API detour_interface funhook_detour_impl_interface_singleton(void);

DYNLINK_SYMBOL_EXPORT(funhook_detour_impl_interface_singleton);

/**
*  @brief
*    Provide the module information
*
*  @return
*    Static string containing module information
*
*/
FUNHOOK_DETOUR_API const char * funhook_detour_print_info(void);

DYNLINK_SYMBOL_EXPORT(funhook_detour_print_info);

#ifdef __cplusplus
}
#endif

#endif /* FUNHOOK_DETOUR_H */
