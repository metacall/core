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

#ifndef FUNCHOOK_DETOUR_H
#define FUNCHOOK_DETOUR_H 1

/* -- Headers -- */

#include <funchook_detour/funchook_detour_api.h>

#include <detour/detour_interface.h>

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

/**
*  @brief
*    Provide the module information
*
*  @return
*    Static string containing module information
*
*/
FUNCHOOK_DETOUR_API const char *funchook_detour_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* FUNCHOOK_DETOUR_H */
