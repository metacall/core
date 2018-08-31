/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

#ifndef FUNCHOOK_DETOUR_H
#define FUNCHOOK_DETOUR_H 1

/* -- Headers -- */

#include <funchook_detour/funchook_detour_api.h>

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
FUNCHOOK_DETOUR_API detour_interface funchook_detour_impl_interface_singleton(void);

DYNLINK_SYMBOL_EXPORT(funchook_detour_impl_interface_singleton);

/**
*  @brief
*    Provide the module information
*
*  @return
*    Static string containing module information
*
*/
FUNCHOOK_DETOUR_API const char * funchook_detour_print_info(void);

DYNLINK_SYMBOL_EXPORT(funchook_detour_print_info);

#ifdef __cplusplus
}
#endif

#endif /* FUNCHOOK_DETOUR_H */
