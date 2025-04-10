/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
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

#ifndef DYNLINK_INTERFACE_H
#define DYNLINK_INTERFACE_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#include <dynlink/dynlink_type.h>

#if defined(WIN32) || defined(_WIN32)
	#include <dynlink/dynlink_impl_win32.h>
#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	(((defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)) && (defined(MAC_OS_X_VERSION_10_15) && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_15)))
	#include <dynlink/dynlink_impl_unix.h>
#elif (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
	#include <dynlink/dynlink_impl_macos.h>
#elif defined(__HAIKU__) || defined(__BEOS__)
	#include <dynlink/dynlink_impl_beos.h>
#else
	#error "Unsupported platform for dynlink"
#endif

#include <dynlink/dynlink_impl.h>

#include <preprocessor/preprocessor_concatenation.h>
#include <preprocessor/preprocessor_stringify.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Type definitions -- */

typedef dynlink_symbol_addr *dynlink_symbol_addr_ptr;

typedef const char *(*dynlink_impl_interface_prefix)(void);
typedef const char *(*dynlink_impl_interface_extension)(void);
typedef dynlink_impl (*dynlink_impl_interface_load)(dynlink);
typedef int (*dynlink_impl_interface_symbol)(dynlink, dynlink_impl, const char *, dynlink_symbol_addr_ptr);
typedef int (*dynlink_impl_interface_unload)(dynlink, dynlink_impl);

struct dynlink_impl_interface_type
{
	dynlink_impl_interface_prefix prefix;
	dynlink_impl_interface_extension extension;
	dynlink_impl_interface_load load;
	dynlink_impl_interface_symbol symbol;
	dynlink_impl_interface_unload unload;
};

typedef dynlink_impl_interface (*dynlink_impl_interface_singleton_ptr)(void);

/* -- Methods -- */

/**
*  @brief
*    Get the dynlink interface for specified platform
*
*  @return
*    A constant pointer to the platform singleton
*/
DYNLINK_API dynlink_impl_interface_singleton_ptr dynlink_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_INTERFACE_H */
